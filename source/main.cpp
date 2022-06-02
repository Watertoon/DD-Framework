 /*
 *  Copyright (C) W. Michael Knudson
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License along with this program; 
 *  if not, write to the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */
#include <dd.hpp>

namespace {

    dd::util::TypeStorage<dd::vk::Context>       context;
    dd::util::TypeStorage<dd::vk::DisplayBuffer> framebuffer;
    dd::util::TypeStorage<dd::vk::CommandBuffer> command_buffers[dd::vk::DisplayBuffer::FramesInFlight];

    struct ContextInitState {
        bool    is_ready_for_exit;
        HANDLE  context_event;
        SRWLOCK context_lock;
    };
}

long unsigned int ContextMain(void *arg) {
    
    /* Create Vulkan Context */
    dd::hid::InitializeRawInputThread();

    ContextInitState *context_state = reinterpret_cast<ContextInitState*>(arg);

    dd::util::ConstructAt(context);
    dd::util::GetReference(context).EndResizeManuel();

    dd::util::ConstructAt(framebuffer);
    dd::util::GetReference(framebuffer).Initialize(dd::util::GetPointer(context));
    
    for (u32 i = 0; i < dd::vk::DisplayBuffer::FramesInFlight; ++i) {
        dd::util::ConstructAt(command_buffers[i]);
        dd::util::GetReference(command_buffers[i]).Initialize(dd::util::GetPointer(context));
    }

    dd::vk::SetGlobalContext(dd::util::GetPointer(context));

    ::SetEvent(context_state->context_event);

    /* Process Vulkan Window */
    MSG msg = {};
    while (::GetMessage(std::addressof(msg), nullptr, 0, 0) != 0) {
        ::DispatchMessage(std::addressof(msg));
        dd::vk::GetGlobalContext()->EndResizeIfNecessary();
    }

    /* End main loop in main thread */
    ::AcquireSRWLockExclusive(std::addressof(context_state->context_lock));
    context_state->is_ready_for_exit = true;
    ::ReleaseSRWLockExclusive(std::addressof(context_state->context_lock));

    /* Wait for main thread to finish */
    ::WaitForSingleObject(context_state->context_event, INFINITE);
    ::ResetEvent(context_state->context_event);

    /* Cleanup */
    ::pfn_vkQueueWaitIdle(dd::util::GetPointer(context)->GetGraphicsQueue());
    ::pfn_vkDeviceWaitIdle(dd::util::GetPointer(context)->GetDevice());

    dd::util::GetReference(framebuffer).Finalize(dd::util::GetPointer(context));
    dd::util::DestructAt(framebuffer);

    for (u32 i = 0; i < dd::vk::DisplayBuffer::FramesInFlight; ++i) {
        dd::util::GetReference(command_buffers[i]).Finalize(dd::util::GetPointer(context));
        dd::util::DestructAt(command_buffers[i]);
    }

    dd::vk::SetGlobalContext(nullptr);
    dd::util::DestructAt(context);

    dd::hid::FinalizeRawInputThread();
    return 0;
}

void Draw(dd::vk::Context *global_context, dd::vk::CommandBuffer *global_command_buffer, dd::util::DelegateThread *present_thread) {

    global_context->LockWindowResize();

    if (global_context->IsSkipDrawUnsafe() == true) {
        global_context->UnlockWindowResize();
        return;
    }

    dd::vk::DisplayBuffer *global_frame_buffer = dd::util::GetPointer(framebuffer);

    /* Get current targets */
    dd::vk::ColorTargetView *current_color_target = global_frame_buffer->GetCurrentColorTarget();
    dd::vk::DepthStencilTargetView *depth_stencil_target = global_frame_buffer->GetDepthStencilTarget();
    dd::vk::Texture *color_target_texture = current_color_target->GetTexture();
    dd::vk::Texture *depth_target_texture = depth_stencil_target->GetTexture();

    /* Register clear colors*/
    global_command_buffer->RegisterFastClearColor({0.0f, 0.0f, 0.0f, 1.0f});
    global_command_buffer->RegisterFastClearDepthStencil({0.0f, 0});

    global_command_buffer->Begin();
    global_context->EnterDraw();

    /* Transition render targets to attachment */
    const dd::vk::TextureBarrierCmdState clear_barrier_state = {
        .vk_src_stage_mask  = 0,
        .vk_dst_stage_mask  = VK_PIPELINE_STAGE_TRANSFER_BIT,
        .vk_dst_access_mask = VK_ACCESS_TRANSFER_WRITE_BIT,
        .vk_src_layout      = VK_IMAGE_LAYOUT_UNDEFINED,
        .vk_dst_layout      = VK_IMAGE_LAYOUT_GENERAL
    };
    global_command_buffer->SetTextureStateTransition(color_target_texture, std::addressof(clear_barrier_state), VK_IMAGE_ASPECT_COLOR_BIT);
    global_command_buffer->SetTextureStateTransition(depth_target_texture, std::addressof(clear_barrier_state), static_cast<VkImageAspectFlagBits>(VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT));

    /* Set render target */
    global_command_buffer->SetRenderTargets(1 , std::addressof(current_color_target), depth_stencil_target);

    /* Draw Frame */
    dd::learn::DrawTriangle(global_command_buffer);

    /* Transition render target to present */
    const dd::vk::TextureBarrierCmdState present_barrier_state = {
        .vk_src_stage_mask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .vk_dst_stage_mask  = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
        .vk_src_access_mask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        .vk_src_layout      = VK_IMAGE_LAYOUT_GENERAL,
        .vk_dst_layout      = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
    };
    global_command_buffer->SetTextureStateTransition(color_target_texture, std::addressof(present_barrier_state), VK_IMAGE_ASPECT_COLOR_BIT);

    /* End draw */
    global_command_buffer->End();

    /* Send present message */
    present_thread->SendMessage(reinterpret_cast<size_t>(global_command_buffer));

    global_context->UnlockWindowResize();
}

int main() {
    /* Initialize System Time */
    dd::util::InitializeTime();

    /* Set flush denormals to 0 */
    _MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);

    /* Create Vulkan Context Thread and wait for initialization */
    ContextInitState context_init_state = {};
    context_init_state.context_event = ::CreateEvent(nullptr, true, false, nullptr);
    DD_ASSERT(context_init_state.context_event != nullptr);

    Handle window_thread = ::CreateThread(nullptr, 0x3000, ContextMain, std::addressof(context_init_state), 0, nullptr);
    DD_ASSERT(window_thread != nullptr);
    ::WaitForSingleObject(context_init_state.context_event, INFINITE);
    ::ResetEvent(context_init_state.context_event);

    dd::learn::SetupTriangle();

    /* Setup for main loop */
    dd::vk::Context         *global_context = dd::vk::GetGlobalContext();
    dd::vk::CommandBuffer   *global_command_buffer = dd::util::GetPointer(command_buffers[dd::util::GetPointer(framebuffer)->GetCurrentFrame()]);
    dd::util::DelegateThread *present_thread = global_context->InitializePresentationThread(dd::util::GetPointer(framebuffer));

    /* Calc And Draw */
    while (true) {

        /* Check for user exit */
        ::AcquireSRWLockExclusive(std::addressof(context_init_state.context_lock));
        if (context_init_state.is_ready_for_exit == true) {
            break;
        }
        ::ReleaseSRWLockExclusive(std::addressof(context_init_state.context_lock));

        /* Begin frame */
        dd::util::BeginFrame();
        dd::hid::BeginFrame();

        Draw(global_context, global_command_buffer, present_thread);

        /* Calc Frame */
        dd::learn::CalcTriangle();

        /* Wait for presentation */
        global_context->WaitForGpu();

        global_command_buffer = dd::util::GetPointer(command_buffers[dd::util::GetPointer(framebuffer)->GetCurrentFrame()]);
    }

    present_thread->FinalizeThread();

    ::pfn_vkQueueWaitIdle(dd::util::GetPointer(context)->GetGraphicsQueue());
    ::pfn_vkDeviceWaitIdle(dd::util::GetPointer(context)->GetDevice());

    /* Cleanup */
    dd::learn::CleanTriangle();

    /* Signal Window thread we are finished */
    ::SetEvent(context_init_state.context_event);

    /* Wait For Window thread to close */
    ::puts("waiting for window exit\n");
    ::WaitForSingleObject(window_thread, INFINITE);

    /* Cleanup*/
    ::CloseHandle(context_init_state.context_event);
    return 0;
}
