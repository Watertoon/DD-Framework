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

    class DisplayBuffer {
        public:
            static constexpr u32 FramesInFlight = 3;
            static constexpr u32 ColorTargetPerFrame = 1;
            static constexpr u32 DepthStencilTargetPerFrame = 1;
        private:
            VkSwapchain             m_vk_swapchain;
            MemoryPool             *m_texture_memory;
            Texture                 m_textures[FramesInFlight * (ColorTargetPerFrame + DepthStencilTargetPerFrame)];
            ColorTargetView         m_color_targets[FramesInFlight * ColorTargetPerFrame];
            DepthStencilTargetView  m_depth_stencil_targets[FramesInFlight * DepthStencilTargetPerFrame];
        public:
            Framebuffer() {/*...*/}

            void Initialize(const Context *context, MemoryPool *texture_memory) {
                /* Create textures */
                const TextureInfo color_texure_info = {
                    
                };
                
                for (u32 i = 0; i < FramesInFlight * ColorTargetPerFrame; ++i) {
                    m_textures[i].Initialize(context, std::addressof(color_texure_info));
                }
                
                /* Create Swapchain */
                const u32 family_index = Context->GetGraphicsQueueFamilyIndex();
                const VkSwapchainCreateInfoKHR swapchain_info = {
                    .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO,
                    .surface = context->GetSurface(),
                    .minImageCount = 3,
                    .imageFormat = Context::TargetSurfaceFormat.format,
                    .imageColorSpace = Context::TargetSurfaceFormat.colorSpace,
                    .imageExtent = Context::GetWindowExtent(),
                    .imageArrayLayers = 1,
                    .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                    .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
                    .queueFamilyIndex = 1,
                    .pQueueFamilyIndices = std::addressof(family_index),
                    .presentMode = Context::TargetPresentMode
                };

                const u32 result = ::vkCreateSwapchainKHR(context->GetDevice(), std::addressof(swapchain_info), nullptr, std::addressof(m_vk_swapchain));
                DD_ASSERT(result != 0);
            }

            void Finalize(const Context *context) {
                ::vkDestroySwapchainKHR(Context->GetDevice(), m_vk_swapchain, nullptr);
            }

            void OnResize() {
                
            }
            
            u64 GetRequiredTextureMemory() {
                /* Initialize staging images */
                
                /* Query */
                
                /* Cleanup */
            }
    };
}