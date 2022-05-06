#include <dd.hpp>

namespace {
    dd::util::TypeStorage<dd::vk::Context>       context;
    dd::util::TypeStorage<dd::vk::FrameBuffer>   framebuffer;
    dd::util::TypeStorage<dd::vk::CommandBuffer> command_buffer;
    
    struct ContextInitState {
        bool    is_ready_for_exit;
        HANDLE  context_event;
        SRWLOCK context_lock;
    };
}

long unsigned int ContextMain(void *arg) {
    
    /* Create Vulkan Context */
    ContextInitState *context_state = reinterpret_cast<ContextInitState*>(arg);
    dd::util::ConstructAt(context);
    dd::util::ConstructAt(framebuffer);
    dd::util::GetReference(framebuffer).Initialize(dd::util::GetPointer(context));
    dd::util::ConstructAt(command_buffer);
    dd::util::GetReference(command_buffer).Initialize(dd::util::GetPointer(context));

    dd::vk::SetGlobalContext(dd::util::GetPointer(context));

    ::SetEvent(context_state->context_event);

    /* Process Vulkan Window */
    MSG msg = {};
    while (::GetMessage(std::addressof(msg), nullptr, 0, 0) != 0) {
        ::TranslateMessage(std::addressof(msg));
        ::DispatchMessage(std::addressof(msg));
    }
    
    /* End main loop in main thread */
    ::AcquireSRWLockExclusive(std::addressof(context_state->context_lock));
    context_state->is_ready_for_exit = true;
    ::ReleaseSRWLockExclusive(std::addressof(context_state->context_lock));
    
    /* Wait for main thread to finish */
    ::WaitForSingleObject(context_state->context_event, INFINITE);
    
    /* Cleanup */
    dd::vk::SetGlobalContext(nullptr);
    dd::util::GetReference(framebuffer).Finalize(dd::util::GetPointer(context));
    dd::util::DestructAt(framebuffer);
    dd::util::GetReference(command_buffer).Finalize(dd::util::GetPointer(context));
    dd::util::DestructAt(command_buffer);
    dd::util::DestructAt(context);
    return 0;
}

int main() {
    /* Initialize System Time */
    dd::util::InitializeTime();
    dd::util::SetFrameFrequency(60);
    
    /* Set flush denormals to 0 */
    _MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);

    /* Create Vulkan Context Thread and wait for initialization */
    ContextInitState context_init_state = {};
    context_init_state.context_event = ::CreateEvent(nullptr, true, false, nullptr);
    DD_ASSERT(context_init_state.context_event != nullptr);

    Handle window_thread = ::CreateThread(nullptr, 0x3000, ContextMain, std::addressof(context_init_state), 0, nullptr);
    DD_ASSERT(window_thread != nullptr);
    ::WaitForSingleObject(context_init_state.context_event, INFINITE);

    dd::learn::SetupTriangle();
    
    /* Setup for main loop */
    dd::vk::Context         *global_context = dd::vk::GetGlobalContext();
    dd::vk::CommandBuffer   *global_command_buffer = dd::util::GetPointer(command_buffer);
    dd::vk::FrameBuffer     *global_frame_buffer = dd::util::GetPointer(framebuffer);
    dd::vk::ColorTargetView *current_color_target = global_frame_buffer->GetCurrentColorTarget();

    const VkClearColorValue clear_color = {
        .float32 = { 0.0f, 1.0f, 1.0f, 1.0f }
    };
    const VkImageSubresourceRange clear_sub_range = {
        .levelCount = 1,
        .layerCount = 1,
    };

    VkFence submit_fence;

    {
        const VkFenceCreateInfo fence_info = {
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO
        };
        const u32 result0 = ::vkCreateFence(global_context->GetDevice(), std::addressof(fence_info), nullptr, std::addressof(submit_fence));
        DD_ASSERT(result0 == VK_SUCCESS);
    }
    
    
    /* Calc And Draw */
    dd::hid::InitializeRawInputThread(global_context->GetWindowHandle());
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
        global_command_buffer->Begin();
        
        /* Transition render target to attachment */
        const dd::vk::TextureBarrierCmdState attachment_barrier_state = {
            .vk_dst_access_mask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            .vk_src_layout      = VK_IMAGE_LAYOUT_UNDEFINED,
            .vk_dst_layout      = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        };
        global_command_buffer->SetTextureStateTransition(current_color_target->GetTexture(), std::addressof(attachment_barrier_state), VK_IMAGE_ASPECT_COLOR_BIT);
        
        /* Clear render target */
        global_command_buffer->ClearColorTarget(current_color_target, std::addressof(clear_color), std::addressof(clear_sub_range));
        
        /* Set render target */
        global_command_buffer->SetRenderTargets(1 , std::addressof(current_color_target), nullptr);

        /* Draw */
        dd::learn::DrawTriangle(global_command_buffer);
        
        /* Transition render target to present */
        const dd::vk::TextureBarrierCmdState present_barrier_state = {
            .vk_src_access_mask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            .vk_src_layout      = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .vk_dst_layout      = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
        };
        global_command_buffer->SetTextureStateTransition(current_color_target->GetTexture(), std::addressof(present_barrier_state), VK_IMAGE_ASPECT_COLOR_BIT);
        
        /* End draw */
        global_command_buffer->End();

        /* Submit command buffer */
        VkCommandBuffer vk_command_buffer = global_command_buffer->GetCommandBuffer();
        const VkSubmitInfo submit_info = {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .commandBufferCount = 1,
            .pCommandBuffers = std::addressof(vk_command_buffer)
        };
        const u32 result1 = ::vkQueueSubmit(global_context->GetGraphicsQueue(), 1, std::addressof(submit_info), submit_fence);
        DD_ASSERT(result1 == VK_SUCCESS);
        
        /* Present */
        global_frame_buffer->PresentTextureAndAcquireNext(global_context);

        /* Calc */
        dd::learn::CalcTriangle();
    }
    dd::hid::FinalizeRawInputThread();

    /* Cleanup */
    dd::learn::CleanTriangle();
    ::vkDestroyFence(dd::vk::GetGlobalContext()->GetDevice(), submit_fence, nullptr);

    /* Signal Window thread we are finished */
    ::SetEvent(context_init_state.context_event);

    /* Wait For Window thread to close */
    std::cout << "waiting for window exit" << std::endl;
    ::WaitForSingleObject(window_thread, INFINITE);

    /* Cleanup*/
    ::CloseHandle(context_init_state.context_event);
    return 0;
}
