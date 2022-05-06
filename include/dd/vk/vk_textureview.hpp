#pragma once

namespace dd::vk {

    struct TextureViewInfo {
        u8       vk_swizzle_r;
        u8       vk_swizzle_g;
        u8       vk_swizzle_b;
        u8       vk_swizzle_a;
        u16      vk_aspect_mask;
        u16      vk_view_type;
        u16      base_mip;
        u16      mip_levels;
        u16      base_array;
        u16      array_levels;
        u32      vk_format;
        Texture *texture;

        constexpr void SetDefaults() {
            vk_swizzle_r   = VK_COMPONENT_SWIZZLE_IDENTITY;
            vk_swizzle_g   = VK_COMPONENT_SWIZZLE_IDENTITY;
            vk_swizzle_b   = VK_COMPONENT_SWIZZLE_IDENTITY;
            vk_swizzle_a   = VK_COMPONENT_SWIZZLE_IDENTITY;
            vk_aspect_mask = VK_IMAGE_ASPECT_COLOR_BIT;
            vk_view_type   = VK_IMAGE_VIEW_TYPE_2D;
            base_mip       = 0;
            mip_levels     = 1;
            base_array     = 0;
            array_levels   = 1;
            vk_format      = VK_FORMAT_UNDEFINED;
            texture        = nullptr;
        }
    };

    class TextureView {
        private:
            VkImageView  m_vk_image_view;
            Texture     *m_parent_texture;
        public:
            constexpr TextureView() {/*...*/}

            void Initialize(const Context *context, const TextureViewInfo *texture_view_info) {

                /* Create image view */
                const VkImageViewCreateInfo image_view_info = {
                    .sType    = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                    .image    = texture_view_info->texture->GetImage(),
                    .viewType = static_cast<VkImageViewType>(texture_view_info->vk_view_type),
                    .format   = static_cast<VkFormat>(texture_view_info->vk_format),
                    .components = {
                        .r = static_cast<VkComponentSwizzle>(texture_view_info->vk_swizzle_r),
                        .g = static_cast<VkComponentSwizzle>(texture_view_info->vk_swizzle_g),
                        .b = static_cast<VkComponentSwizzle>(texture_view_info->vk_swizzle_b),
                        .a = static_cast<VkComponentSwizzle>(texture_view_info->vk_swizzle_a)
                    },
                    .subresourceRange = {
                        .aspectMask     = texture_view_info->vk_aspect_mask,
                        .baseMipLevel   = texture_view_info->base_mip,
                        .levelCount    = texture_view_info->mip_levels,
                        .baseArrayLayer = texture_view_info->base_array,
                        .layerCount     = texture_view_info->array_levels
                    }
                };

                const u32 result0 = ::vkCreateImageView(context->GetDevice(), std::addressof(image_view_info), nullptr, std::addressof(m_vk_image_view));
                DD_ASSERT(result0 == VK_TRUE);

                m_parent_texture = texture_view_info->texture;
            }

            void Finalize(const Context *context) {
                ::vkDestroyImageView(context->GetDevice(), m_vk_image_view, nullptr);
            }

            constexpr VkImageView GetImageView() const  { return m_vk_image_view; }

            constexpr Texture *GetTexture()             { return m_parent_texture; }

            constexpr const Texture *GetTexture() const { return m_parent_texture; }
    };
}
