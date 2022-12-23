 /*
 *  Copyright (C) W. Michael Knudson
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as 
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License along with this program; 
 *  if not, see <https://www.gnu.org/licenses/>.
 */
#pragma once

namespace dd::vk {

    class DisplayBuffer : public util::LogicalFrameBuffer {
        public:
            static constexpr u32 FramesInFlight = 2;
            static constexpr u32 BufferedFrames = 3;
            static constexpr u32 ColorTargetPerFrame = 1;
            static constexpr u32 DepthStencilTargetPerFrame = 1;
        private:
            VkSwapchainKHR         m_vk_swapchain;
            VkSemaphore            m_vk_queue_present_semaphore[FramesInFlight];
            VkFence                m_vk_queue_submit_fence[FramesInFlight];
            VkFence                m_vk_image_acquire_fence;
            Texture                m_vk_swapchain_textures[BufferedFrames];
            ColorTargetView        m_vk_swapchain_targets[BufferedFrames];
            VkDeviceMemory         m_vk_depth_stencil_image_memory;
            MemoryPool             m_depth_stencil_memory;
            Texture                m_depth_stencil_texture;
            DepthStencilTargetView m_depth_stencil_target;
            u32                    m_current_target_index;
            u32                    m_current_frame;
        public:
            constexpr DisplayBuffer() {/*...*/}

            void Initialize(Context *context) {

                /* Create Swapchain */
                u32 window_width = 0, window_height = 0;
                context->GetWindowDimensionsUnsafe(std::addressof(window_width), std::addressof(window_height));

                this->SetVirtualCanvasSize(static_cast<float>(window_width), static_cast<float>(window_height));
                this->SetDimensions(static_cast<float>(window_width), static_cast<float>(window_height));
            
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

                const u32 result0 = ::pfn_vkCreateSwapchainKHR(context->GetDevice(), std::addressof(swapchain_info), nullptr, std::addressof(m_vk_swapchain));
                DD_ASSERT(result0 == VK_SUCCESS);

                /* Initialize textures */
                VkImage images[BufferedFrames] = {};
                u32 image_count = BufferedFrames;
                const u32 result1 = ::pfn_vkGetSwapchainImagesKHR(context->GetDevice(), m_vk_swapchain, std::addressof(image_count), images);
                DD_ASSERT(result1 == VK_SUCCESS);

                for (u32 i = 0; i < BufferedFrames; ++i) {
                    m_vk_swapchain_textures[i].ImportExisting(images[i]);
                }

                /* Initialize ColorTargets */
                ColorTargetInfo target_info = {
                    .vk_image_view_type = VK_IMAGE_VIEW_TYPE_2D,
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
                    const u32 result10 = ::pfn_vkCreateSemaphore(context->GetDevice(), std::addressof(semaphore_info), nullptr, std::addressof(m_vk_queue_present_semaphore[i]));
                    DD_ASSERT(result10 == VK_SUCCESS);
                }

                const u32 result3 = ::pfn_vkCreateFence(context->GetDevice(), std::addressof(fence_info), nullptr, std::addressof(m_vk_image_acquire_fence));
                DD_ASSERT(result3 == VK_SUCCESS);

                fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

                for (u32 i = 0; i < FramesInFlight; ++i) {
                    const u32 result6 = ::pfn_vkCreateFence(context->GetDevice(), std::addressof(fence_info), nullptr, std::addressof(m_vk_queue_submit_fence[i]));
                    DD_ASSERT(result6 == VK_SUCCESS);
                }

                /* Acquire first image index */
                const u32 result7 = ::pfn_vkAcquireNextImageKHR(context->GetDevice(), m_vk_swapchain, 0, VK_NULL_HANDLE, m_vk_image_acquire_fence, std::addressof(m_current_target_index));
                DD_ASSERT(result7 == VK_SUCCESS);

                const u32 result8 = ::pfn_vkWaitForFences(context->GetDevice(), 1, std::addressof(m_vk_image_acquire_fence), VK_TRUE, UINT64_MAX);
                DD_ASSERT(result8 == VK_SUCCESS);

                /* Create depth image */
                const TextureInfo depth_image_info = {
                    .vk_usage_flags = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                    .mip_levels = 1,
                    .width = window_width,
                    .height = window_height,
                    .depth = 1,
                    .vk_format = Context::TargetDepthStencilFormat,
                    .vk_image_layout = VK_IMAGE_LAYOUT_UNDEFINED,
                    .vk_image_type = VK_IMAGE_TYPE_2D,
                    .vk_sample_count_flag = VK_SAMPLE_COUNT_1_BIT,
                    .vk_tiling = VK_IMAGE_TILING_OPTIMAL,
                    .array_layers = 1
                };
                const u64 required_memory = util::AlignUp(Texture::GetRequiredMemorySize(context, std::addressof(depth_image_info)), Context::TargetMemoryPoolAlignment);

                const s32 memory_type = context->FindMemoryHeapIndex(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
                const VkMemoryAllocateInfo depth_stencil_allocate_info =  {
                    .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
                    .allocationSize = required_memory,
                    .memoryTypeIndex = static_cast<u32>(memory_type)
                };
                const u32 result9 = ::pfn_vkAllocateMemory(context->GetDevice(), std::addressof(depth_stencil_allocate_info), nullptr, std::addressof(m_vk_depth_stencil_image_memory));
                DD_ASSERT(result9 == VK_SUCCESS);
                
                m_depth_stencil_memory.ImportExisting(m_vk_depth_stencil_image_memory, true);

                m_depth_stencil_texture.Initialize(context, std::addressof(depth_image_info), std::addressof(m_depth_stencil_memory));
                
                const DepthStencilTargetInfo depth_target_info = {
                    .vk_image_view_type = VK_IMAGE_VIEW_TYPE_2D,
                    .vk_format = Context::TargetDepthStencilFormat,
                    .base_layer = 0,
                    .layers = 1,
                    .base_mip_level = 0,
                    .mip_levels = 1,
                    .vk_swizzle_r = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .vk_swizzle_g = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .vk_swizzle_b = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .vk_swizzle_a = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .depth_stencil_attachment = std::addressof(m_depth_stencil_texture)
                };
                m_depth_stencil_target.Initialize(context, std::addressof(depth_target_info));
            }

            void Finalize(const Context *context) {

                ::pfn_vkDestroyFence(context->GetDevice(), m_vk_image_acquire_fence, nullptr);
                for (u32 i = 0; i < FramesInFlight; ++i) {
                    ::pfn_vkDestroySemaphore(context->GetDevice(), m_vk_queue_present_semaphore[i], nullptr);
                    ::pfn_vkDestroyFence(context->GetDevice(), m_vk_queue_submit_fence[i], nullptr);
                }

                for (u32 i = 0; i < BufferedFrames; ++i) {
                    m_vk_swapchain_targets[i].Finalize(context);
                    m_vk_swapchain_textures[i].Finalize(context);
                }

                m_depth_stencil_target.Finalize(context);
                m_depth_stencil_texture.Finalize(context);
                m_depth_stencil_memory.Finalize(context);
                ::pfn_vkFreeMemory(context->GetDevice(), m_vk_depth_stencil_image_memory, nullptr);

                m_current_frame = 0;
            }

            void Recreate(const Context *context) {

                /* Create new Swapchain */
                VkSwapchainKHR old_swapchain = m_vk_swapchain;
                u32 window_width = 0, window_height = 0;
                context->GetWindowDimensionsUnsafe(std::addressof(window_width), std::addressof(window_height));

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
                    .clipped = VK_TRUE,
                    .oldSwapchain = old_swapchain
                };

                const VkDevice vk_device = context->GetDevice();
                const u32 result0 = ::pfn_vkCreateSwapchainKHR(vk_device, std::addressof(swapchain_info), nullptr, std::addressof(m_vk_swapchain));
                DD_ASSERT(result0 == VK_SUCCESS);

                /* Reset state */
                ::pfn_vkDestroySwapchainKHR(vk_device, old_swapchain, nullptr);

                for (u32 i = 0; i < BufferedFrames; ++i) {
                    m_vk_swapchain_targets[i].Finalize(context);
                    m_vk_swapchain_textures[i].Finalize(context);
                }

                m_current_frame = 0;

                /* Reinitialize textures */
                VkImage images[BufferedFrames] = {};
                u32 image_count = BufferedFrames;
                const u32 result1 = ::pfn_vkGetSwapchainImagesKHR(vk_device, m_vk_swapchain, std::addressof(image_count), images);
                DD_ASSERT(result1 == VK_SUCCESS);

                for (u32 i = 0; i < BufferedFrames; ++i) {
                    m_vk_swapchain_textures[i].ImportExisting(images[i]);
                }

                /* Reinitialize ColorTargets */
                ColorTargetInfo target_info = {
                    .vk_image_view_type = VK_IMAGE_VIEW_TYPE_2D,
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

                /* Acquire first image index */
                const u32 result9 = ::pfn_vkResetFences(vk_device, 1, std::addressof(m_vk_image_acquire_fence));
                DD_ASSERT(result9 == VK_SUCCESS);

                const u32 result7 = ::pfn_vkAcquireNextImageKHR(vk_device, m_vk_swapchain, 0, VK_NULL_HANDLE, m_vk_image_acquire_fence, std::addressof(m_current_target_index));
                DD_ASSERT(result7 == VK_SUCCESS);

                /* Recreate Semaphores */
                for (u32 i = 0; i < FramesInFlight; ++i) {
                    ::pfn_vkDestroySemaphore(vk_device, m_vk_queue_present_semaphore[i], nullptr);
                }
                
                VkSemaphoreCreateInfo semaphore_info = {
                    .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
                };

                for (u32 i = 0; i < FramesInFlight; ++i) {
                    const u32 result10 = ::pfn_vkCreateSemaphore(vk_device, std::addressof(semaphore_info), nullptr, std::addressof(m_vk_queue_present_semaphore[i]));
                    DD_ASSERT(result10 == VK_SUCCESS);
                }

                /* Rereate depth image */
                m_depth_stencil_target.Finalize(context);
                m_depth_stencil_texture.Finalize(context);
                m_depth_stencil_memory.Finalize(context);
                ::pfn_vkFreeMemory(context->GetDevice(), m_vk_depth_stencil_image_memory, nullptr);

                const TextureInfo depth_image_info = {
                    .vk_usage_flags = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                    .mip_levels = 1,
                    .width = window_width,
                    .height = window_height,
                    .depth = 1,
                    .vk_format = Context::TargetDepthStencilFormat,
                    .vk_image_layout = VK_IMAGE_LAYOUT_UNDEFINED,
                    .vk_image_type = VK_IMAGE_TYPE_2D,
                    .vk_sample_count_flag = VK_SAMPLE_COUNT_1_BIT,
                    .vk_tiling = VK_IMAGE_TILING_OPTIMAL,
                    .array_layers = 1
                };
                const u64 required_memory = util::AlignUp(Texture::GetRequiredMemorySize(context, std::addressof(depth_image_info)), Context::TargetMemoryPoolAlignment);

                const s32 memory_type = context->FindMemoryHeapIndex(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
                const VkMemoryAllocateInfo depth_stencil_allocate_info =  {
                    .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
                    .allocationSize = required_memory,
                    .memoryTypeIndex = static_cast<u32>(memory_type)
                };
                const u32 result11 = ::pfn_vkAllocateMemory(context->GetDevice(), std::addressof(depth_stencil_allocate_info), nullptr, std::addressof(m_vk_depth_stencil_image_memory));
                DD_ASSERT(result11 == VK_SUCCESS);

                m_depth_stencil_memory.ImportExisting(m_vk_depth_stencil_image_memory, true);

                m_depth_stencil_texture.Initialize(context, std::addressof(depth_image_info), std::addressof(m_depth_stencil_memory));

                const DepthStencilTargetInfo depth_target_info = {
                    .vk_image_view_type = VK_IMAGE_VIEW_TYPE_2D,
                    .vk_format = Context::TargetDepthStencilFormat,
                    .base_layer = 0,
                    .layers = 1,
                    .base_mip_level = 0,
                    .mip_levels = 1,
                    .vk_swizzle_r = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .vk_swizzle_g = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .vk_swizzle_b = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .vk_swizzle_a = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .depth_stencil_attachment = std::addressof(m_depth_stencil_texture)
                };
                m_depth_stencil_target.Initialize(context, std::addressof(depth_target_info));
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
                const u32 result0 = ::pfn_vkQueuePresentKHR(context->GetGraphicsQueue(), std::addressof(present_info));
                DD_ASSERT(result0 == VK_SUCCESS);

                /* Acquire next image index */
                const VkDevice vk_device = context->GetDevice();
                const u32 result1 = ::pfn_vkResetFences(vk_device, 1, std::addressof(m_vk_image_acquire_fence));
                DD_ASSERT(result1 == VK_SUCCESS);
                
                const u32 result2 = ::pfn_vkAcquireNextImageKHR(vk_device, m_vk_swapchain, 0, VK_NULL_HANDLE, m_vk_image_acquire_fence, std::addressof(m_current_target_index));
                DD_ASSERT(result2 == VK_SUCCESS);

                m_current_frame = (m_current_frame + 1) % FramesInFlight;
            }

            bool ApplyResize(Context *context) {

                context->LockWindowResize();

                if (context->HasWindowResizedUnsafe() == false || context->HasValidWindowDimensionsUnsafe() == false) {
                    context->UnlockWindowResize();
                    return false;
                }

                const VkDevice vk_device = context->GetDevice();

                /* Wait for previous frames to run their course */
                ::pfn_vkQueueWaitIdle(context->GetGraphicsQueue());
                ::pfn_vkDeviceWaitIdle(vk_device);

                /* Recreate swapchain */
                this->Recreate(context);

                context->ClearResizeUnsafe();

                context->UnlockWindowResize();

                return true;
            }

            constexpr ColorTargetView *GetCurrentColorTarget()         { return std::addressof(m_vk_swapchain_targets[m_current_target_index]); }

            constexpr DepthStencilTargetView *GetDepthStencilTarget()         { return std::addressof(m_depth_stencil_target); }

            constexpr VkSemaphore GetCurrentQueuePresentSemaphore()    { return m_vk_queue_present_semaphore[m_current_frame]; }

            void ResetCurrentQueueSubmitFence() {
                const u32 result0 = ::pfn_vkResetFences(GetGlobalContext()->GetDevice(), 1, std::addressof(m_vk_queue_submit_fence[m_current_frame]));
                DD_ASSERT(result0 == VK_SUCCESS);
            }

            constexpr VkFence GetCurrentQueueSubmitFence()             { return m_vk_queue_submit_fence[m_current_frame]; }

            constexpr VkFence GetImageAcquireFence()                   { return m_vk_image_acquire_fence; }

            constexpr u32 GetCurrentFrame() const                      { return m_current_frame; }
    };
}
