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

    struct DepthStencilTargetInfo {
        u8       vk_image_view_type;
        u8       reserve;
        u16      levels;
        u32      vk_format;
        u32      base_layer;
        u32      layers;
        u32      base_mip_level;
        u32      mip_levels;
        u8       vk_swizzle_r;
        u8       vk_swizzle_g;
        u8       vk_swizzle_b;
        u8       vk_swizzle_a;
        Texture *depth_stencil_attachment;
    };

    class DepthStencilTargetView {
        private:
            Texture *m_texture;
            VkImageView m_vk_depth_stencil_image_view;
        public:
            constexpr DepthStencilTargetView() {/*...*/}

            void Initialize(const Context *context, const DepthStencilTargetInfo *target_info) {

                /* Create image view */
                const VkImageViewCreateInfo view_info = {
                    .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                    .image = target_info->depth_stencil_attachment->GetImage(),
                    .viewType = static_cast<VkImageViewType>(target_info->vk_image_view_type),
                    .format = static_cast<VkFormat>(target_info->vk_format),
                    .components = {
                        .r = static_cast<VkComponentSwizzle>(target_info->vk_swizzle_r),
                        .g = static_cast<VkComponentSwizzle>(target_info->vk_swizzle_g),
                        .b = static_cast<VkComponentSwizzle>(target_info->vk_swizzle_b),
                        .a = static_cast<VkComponentSwizzle>(target_info->vk_swizzle_a)
                    },
                    .subresourceRange = {
                        .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,
                        .baseMipLevel = target_info->base_mip_level,
                        .levelCount = target_info->mip_levels,
                        .baseArrayLayer = target_info->base_layer,
                        .layerCount = target_info->layers
                    }
                };

                const u32 result0 = ::vkCreateImageView(context->GetDevice(), std::addressof(view_info), nullptr, std::addressof(m_vk_depth_stencil_image_view));
                DD_ASSERT(result0 == VK_SUCCESS);

                m_texture = target_info->depth_stencil_attachment;
            }

            void Finalize(const Context *context) {
                ::vkDestroyImageView(context->GetDevice(), m_vk_depth_stencil_image_view, nullptr);
                m_vk_depth_stencil_image_view = 0;
            }

            constexpr ALWAYS_INLINE VkImage GetImage() const { return m_texture->GetImage(); }

            constexpr ALWAYS_INLINE VkImageView GetImageView() const { return m_vk_depth_stencil_image_view; }
    };
}