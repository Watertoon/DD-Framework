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
            static constexpr u32 FramesInFlight = 3;
            static constexpr u32 ColorTargetPerFrame = 1;
            static constexpr u32 DepthStencilTargetPerFrame = 1;
        private:
            VkSwapchainKHR   m_vk_swapchain;
            VkSemaphore      m_vk_present_semaphore;
            VkSemaphore      m_vk_acquire_semaphore;
            Texture          m_vk_swapchain_textures[3];
            ColorTargetView  m_vk_swapchain_targets[3];
            u32              m_current_target_index;
        public:
            constexpr FrameBuffer() {/*...*/}

            void Initialize(const Context *context) {

                /* Create Swapchain */
                u32 window_width = 0, window_height = 0;
                context->GetWindowDimensions(std::addressof(window_width), std::addressof(window_height));
                
                const u32 family_index = context->GetGraphicsQueueFamilyIndex();
                const VkSwapchainCreateInfoKHR swapchain_info = {
                    .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
                    .surface = context->GetSurface(),
                    .minImageCount = 3,
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
                DD_ASSERT(result0 != 0);

                /* Initialize textures */
                VkImage images[3] = {};
                u32 image_count = 3;
                const u32 result1 = ::vkGetSwapchainImagesKHR(context->GetDevice(), m_vk_swapchain, std::addressof(image_count), images);
                DD_ASSERT(result1 == VK_SUCCESS);

                m_vk_swapchain_textures[0].ImportExisting(images[0]);
                m_vk_swapchain_textures[1].ImportExisting(images[1]);
                m_vk_swapchain_textures[2].ImportExisting(images[2]);

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
                    .vk_swizzle_a = static_cast<VkComponentSwizzle>(VK_COMPONENT_SWIZZLE_IDENTITY),
                    .color_attachment = std::addressof(m_vk_swapchain_textures[0])
                };

                m_vk_swapchain_targets[0].Initialize(context, std::addressof(target_info));

                target_info.color_attachment = std::addressof(m_vk_swapchain_textures[1]);
                m_vk_swapchain_targets[1].Initialize(context, std::addressof(target_info));

                target_info.color_attachment = std::addressof(m_vk_swapchain_textures[2]);
                m_vk_swapchain_targets[2].Initialize(context, std::addressof(target_info));

                /* Create present sync objects */
                const VkSemaphoreCreateInfo semaphore_info = {
                    .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
                };

                const u32 result2 = ::vkCreateSemaphore(context->GetDevice(), std::addressof(semaphore_info), nullptr, std::addressof(m_vk_present_semaphore));
                DD_ASSERT(result2 == VK_SUCCESS);

                const u32 result3 = ::vkCreateSemaphore(context->GetDevice(), std::addressof(semaphore_info), nullptr, std::addressof(m_vk_acquire_semaphore));
                DD_ASSERT(result3 == VK_SUCCESS);
            }

            void Finalize(const Context *context) {
                ::vkDestroySemaphore(context->GetDevice(), m_vk_present_semaphore, nullptr);
                ::vkDestroySemaphore(context->GetDevice(), m_vk_acquire_semaphore, nullptr);
                m_vk_swapchain_targets[0].Finalize(context);
                m_vk_swapchain_targets[1].Finalize(context);
                m_vk_swapchain_targets[2].Finalize(context);
                m_vk_swapchain_textures[0].Finalize(context);
                m_vk_swapchain_textures[1].Finalize(context);
                m_vk_swapchain_textures[2].Finalize(context);
                ::vkDestroySwapchainKHR(context->GetDevice(), m_vk_swapchain, nullptr);
            }

            void PresentTextureAndAcquireNext(const Context *context) {
                /* Present */
                const VkPresentInfoKHR present_info = {
                    .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
                    .waitSemaphoreCount = 1,
                    .pWaitSemaphores = std::addressof(m_vk_present_semaphore),
                    .swapchainCount = 1,
                    .pSwapchains = std::addressof(m_vk_swapchain),
                    .pImageIndices = std::addressof(m_current_target_index)
                };
                const u32 result0 = ::vkQueuePresentKHR(context->GetGraphicsQueue(), std::addressof(present_info));
                DD_ASSERT(result0 == VK_SUCCESS);

                /* Acquire next image index */
                const u32 result1 = ::vkAcquireNextImageKHR(context->GetDevice(), m_vk_swapchain, UINT64_MAX, m_vk_acquire_semaphore, VK_NULL_HANDLE, std::addressof(m_current_target_index));
                DD_ASSERT(result1 == VK_SUCCESS);;
            }

            void OnResize() {
                
            }
            
            constexpr ColorTargetView *GetCurrentColorTarget() { return std::addressof(m_vk_swapchain_targets[m_current_target_index]); }
    };
}
