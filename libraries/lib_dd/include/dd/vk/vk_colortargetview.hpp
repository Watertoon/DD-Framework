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

    struct ColorTargetInfo {
        u8       vk_image_view_type;
        u8       reserve0;
        u16      reserve1;
        u32      vk_format;
        u32      base_layer;
        u32      layers;
        u32      base_mip_level;
        u32      mip_levels;
        u8       vk_swizzle_r;
        u8       vk_swizzle_g;
        u8       vk_swizzle_b;
        u8       vk_swizzle_a;
        Texture *color_attachment;
    };

    class ColorTargetView {
        private:
            Texture *m_texture;
            VkImageView m_vk_color_image_view;
        public:
            constexpr ColorTargetView() {/*...*/}

            void Initialize(const Context *context, const ColorTargetInfo *target_info) {

                /* Create image view */
                const VkImageViewCreateInfo view_info = {
                    .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                    .image = target_info->color_attachment->GetImage(),
                    .viewType = static_cast<VkImageViewType>(target_info->vk_image_view_type),
                    .format = static_cast<VkFormat>(target_info->vk_format),
                    .components = {
                        .r = static_cast<VkComponentSwizzle>(target_info->vk_swizzle_r),
                        .g = static_cast<VkComponentSwizzle>(target_info->vk_swizzle_g),
                        .b = static_cast<VkComponentSwizzle>(target_info->vk_swizzle_b),
                        .a = static_cast<VkComponentSwizzle>(target_info->vk_swizzle_a)
                    },
                    .subresourceRange = {
                        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                        .baseMipLevel = target_info->base_mip_level,
                        .levelCount = target_info->mip_levels,
                        .baseArrayLayer = target_info->base_layer,
                        .layerCount = target_info->layers
                    }
                };

                const u32 result0 = ::pfn_vkCreateImageView(context->GetDevice(), std::addressof(view_info), nullptr, std::addressof(m_vk_color_image_view));
                DD_ASSERT(result0 == VK_SUCCESS);

                m_texture = target_info->color_attachment;
            }

            void Finalize(const Context *context) {
                ::pfn_vkDestroyImageView(context->GetDevice(), m_vk_color_image_view, nullptr);
                m_vk_color_image_view = 0;
            }

            constexpr ALWAYS_INLINE Texture *GetTexture()            { return m_texture; }

            constexpr ALWAYS_INLINE VkImage GetImage() const         { return m_texture->GetImage(); }

            constexpr ALWAYS_INLINE VkImageView GetImageView() const { return m_vk_color_image_view; }
    };
}
