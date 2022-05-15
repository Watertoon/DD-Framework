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
#pragma once

namespace dd::vk {

    class FrameBuffer {
        public:
            static constexpr u32 FramesInFlight = 2;
            static constexpr u32 BufferedFrames = 3;
            static constexpr u32 ColorTargetPerFrame = 1;
            static constexpr u32 DepthStencilTargetPerFrame = 1;
        private:
            VkSwapchainKHR   m_vk_swapchain;
            VkSemaphore      m_vk_queue_present_semaphore[FramesInFlight];
            VkFence          m_vk_queue_submit_fence[FramesInFlight];
            VkFence          m_vk_image_acquire_fence;
            Texture          m_vk_swapchain_textures[BufferedFrames];
            ColorTargetView  m_vk_swapchain_targets[BufferedFrames];
            u32              m_current_target_index;
            u32              m_current_frame;
            bool             m_requires_resize;
        public:
            constexpr FrameBuffer() {/*...*/}

            void Initialize(Context *context) {

                /* Create Swapchain */
                u32 window_width = 0, window_height = 0;
                context->GetWindowDimensions(std::addressof(window_width), std::addressof(window_height));
                
                const u32 family_index = context->GetGraphicsQueueFamilyIndex();
                const VkSwapchainCreateInfoKHR swapchain_info = {
                    .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
                    .surface = context->GetSurface(),
                    .minImageCount = BufferedFrames,
                    .imageFormat = Context::TargetSurfaceFormat.format,
                    .imageColorSpace = Context::TargetSurfaceFormat.colorSpace,
                    .imageExtent = { 
                        .width = window_width,
                        .height = window_height
                    },
                    .imageArrayLayers = 1,
                    .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                    .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
                    .queueFamilyIndexCount = 1,
                    .pQueueFamilyIndices = std::addressof(family_index),
                    .preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
                    .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
                    .presentMode = Context::TargetPresentMode,
                    .clipped = VK_TRUE
                };

                const u32 result0 = ::vkCreateSwapchainKHR(context->GetDevice(), std::addressof(swapchain_info), nullptr, std::addressof(m_vk_swapchain));
                DD_ASSERT(result0 == VK_SUCCESS);

                /* Initialize textures */
                VkImage images[BufferedFrames] = {};
                u32 image_count = BufferedFrames;
                const u32 result1 = ::vkGetSwapchainImagesKHR(context->GetDevice(), m_vk_swapchain, std::addressof(image_count), images);
                DD_ASSERT(result1 == VK_SUCCESS);
                
                for (u32 i = 0; i < BufferedFrames; ++i) {
                    m_vk_swapchain_textures[i].ImportExisting(images[i]);
                }

                /* Initialize ColorTargets */
                ColorTargetInfo target_info = {
                    .vk_image_view_type = VK_IMAGE_VIEW_TYPE_2D,
                    .levels = 1,
                    .vk_format = Context::TargetSurfaceFormat.format,
                    .base_layer = 0,
                    .layers = 1,
                    .base_mip_level = 0,
                    .mip_levels = 1,
                    .vk_swizzle_r = static_cast<VkComponentSwizzle>(VK_COMPONENT_SWIZZLE_IDENTITY),
                    .vk_swizzle_g = static_cast<VkComponentSwizzle>(VK_COMPONENT_SWIZZLE_IDENTITY),
                    .vk_swizzle_b = static_cast<VkComponentSwizzle>(VK_COMPONENT_SWIZZLE_IDENTITY),
                    .vk_swizzle_a = static_cast<VkComponentSwizzle>(VK_COMPONENT_SWIZZLE_IDENTITY)
                };
                
                for (u32 i = 0; i < BufferedFrames; ++i) {
                    target_info.color_attachment = std::addressof(m_vk_swapchain_textures[i]);
                    m_vk_swapchain_targets[i].Initialize(context, std::addressof(target_info));
                }

                /* Create present sync objects */
                VkSemaphoreCreateInfo semaphore_info = {
                    .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
                };
                VkFenceCreateInfo fence_info = {
                    .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO
                };
                
                for (u32 i = 0; i < FramesInFlight; ++i) {

                    const u32 result10 = ::vkCreateSemaphore(context->GetDevice(), std::addressof(semaphore_info), nullptr, std::addressof(m_vk_queue_present_semaphore[i]));
                    DD_ASSERT(result10 == VK_SUCCESS);
                }
                
                const u32 result3 = ::vkCreateFence(context->GetDevice(), std::addressof(fence_info), nullptr, std::addressof(m_vk_image_acquire_fence));
                DD_ASSERT(result3 == VK_SUCCESS);

                const u32 result4 = ::vkCreateFence(context->GetDevice(), std::addressof(fence_info), nullptr, std::addressof(m_vk_queue_submit_fence[0]));
                DD_ASSERT(result4 == VK_SUCCESS);
                
                fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
                
                for (u32 i = 1; i < FramesInFlight; ++i) {

                    const u32 result6 = ::vkCreateFence(context->GetDevice(), std::addressof(fence_info), nullptr, std::addressof(m_vk_queue_submit_fence[i]));
                    DD_ASSERT(result6 == VK_SUCCESS);
                }
                
                /* Acquire first image index */
                const u32 result7 = ::vkAcquireNextImageKHR(context->GetDevice(), m_vk_swapchain, 0, VK_NULL_HANDLE, m_vk_image_acquire_fence, std::addressof(m_current_target_index));
                DD_ASSERT(result7 == VK_SUCCESS);
                
                const u32 result8 = ::vkWaitForFences(context->GetDevice(), 1, std::addressof(m_vk_image_acquire_fence), VK_TRUE, UINT64_MAX);
                DD_ASSERT(result8 == VK_SUCCESS);

                const u32 result9 = ::vkResetFences(context->GetDevice(), 1, std::addressof(m_vk_image_acquire_fence));
                DD_ASSERT(result9 == VK_SUCCESS);
            }

            void Finalize(const Context *context) {

                ::vkDestroyFence(context->GetDevice(), m_vk_image_acquire_fence, nullptr);
                for (u32 i = 0; i < FramesInFlight; ++i) {
                    ::vkDestroySemaphore(context->GetDevice(), m_vk_queue_present_semaphore[i], nullptr);
                    ::vkDestroyFence(context->GetDevice(), m_vk_queue_submit_fence[i], nullptr);
                }

                for (u32 i = 0; i < BufferedFrames; ++i) {
                    m_vk_swapchain_targets[i].Finalize(context);
                    m_vk_swapchain_textures[i].Finalize(context);
                }

                ::vkDestroySwapchainKHR(context->GetDevice(), m_vk_swapchain, nullptr);
                
                m_current_frame = 0;
                m_requires_resize = false;
            }

            void PresentTextureAndAcquireNext(Context *context) {

                /* Present */
                VkSemaphore current_semaphore = this->GetCurrentQueuePresentSemaphore();
                const VkPresentInfoKHR present_info = {
                    .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
                    .waitSemaphoreCount = 1,
                    .pWaitSemaphores = std::addressof(current_semaphore),
                    .swapchainCount = 1,
                    .pSwapchains = std::addressof(m_vk_swapchain),
                    .pImageIndices = std::addressof(m_current_target_index)
                };
                const u32 result0 = ::vkQueuePresentKHR(context->GetGraphicsQueue(), std::addressof(present_info));
                if (result0 != VK_SUCCESS) {
                    context->SetResize();
                }

                /* Acquire next image index */
                const u32 result1 = ::vkAcquireNextImageKHR(context->GetDevice(), m_vk_swapchain, 0, VK_NULL_HANDLE, m_vk_image_acquire_fence, std::addressof(m_current_target_index));
                if (result1 != VK_SUCCESS) {
                    context->SetResize();
                }

                m_current_frame = (m_current_frame + 1) % FramesInFlight;
            }

            void ApplyResize(Context *context) {

                if (context->HasWindowResized() == false) {
                    return;
                }

                /* Wait for previous frames to run their course */
                VkFence all_fences[FramesInFlight + 1] = {};
                all_fences[0] = m_vk_image_acquire_fence;
                for (u32 i = 1; i < FramesInFlight + 1; ++i) {
                    all_fences[i] = m_vk_queue_submit_fence[i - 1];
                }
                
                ::vkQueueWaitIdle(context->GetGraphicsQueue());

                const u32 result1 = ::vkResetFences(context->GetDevice(), FramesInFlight + 1, all_fences);
                DD_ASSERT(result1 == VK_SUCCESS);

                /* Recreate swapchain */
                this->Finalize(context);
                this->Initialize(context);

                /* Allow the context window to update again */
                context->FinishSwapchainResize();
            }
            
            constexpr ColorTargetView *GetCurrentColorTarget()         { return std::addressof(m_vk_swapchain_targets[m_current_target_index]); }

            constexpr VkSemaphore GetCurrentQueuePresentSemaphore()    { return m_vk_queue_present_semaphore[m_current_frame]; }

            constexpr VkFence GetCurrentQueueSubmitFence()             { return m_vk_queue_submit_fence[m_current_frame]; }

            constexpr VkFence GetImageAcquireFence()                   { return m_vk_image_acquire_fence; }

            constexpr u32 GetCurrentFrame() const                      { return m_current_frame; }
    };
}
