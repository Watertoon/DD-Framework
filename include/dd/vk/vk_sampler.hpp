#pragma once

namespace dd::vk {
    
    struct SamplerInfo {
        u32   vk_filter_min;
        u32   vk_filter_mag;
        u8    vk_mip_map_mode;
        u8    vk_address_mode_u;
        u8    vk_address_mode_v;
        u8    vk_address_mode_w;
        float lod_bias;
        float min_lod_clamp;
        float max_lod_clamp;
        float max_anisotropy_clamp;
        u32   vk_compare_op;
        u32   vk_border_color;
        
        constexpr void SetDefaults() {
            vk_filter_min        = VK_FILTER_NEAREST;
            vk_filter_mag        = VK_FILTER_NEAREST;
            vk_mip_map_mode      = VK_SAMPLER_MIPMAP_MODE_NEAREST;
            vk_address_mode_u    = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            vk_address_mode_v    = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            vk_address_mode_w    = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            lod_bias             = 1.0f;
            min_lod_clamp        = 0.0f;
            max_lod_clamp        = 1.0f;
            max_anisotropy_clamp = 1.0f;
            vk_compare_op        = VK_COMPARE_OP_NEVER;
            vk_border_color      = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
        }  
    };
    
    class Sampler {
        private:
            VkSampler m_vk_sampler;
        public:
            constexpr Sampler() {/*...*/}
            
            void Initialize(const Context *context, const SamplerInfo *sampler_info) {
                
                /* Create sampler */
                const VkSamplerCreateInfo vk_sampler_info = {
                    .sType            = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
                    .magFilter        = static_cast<VkFilter>(sampler_info->vk_filter_mag),
                    .minFilter        = static_cast<VkFilter>(sampler_info->vk_filter_min),
                    .mipmapMode       = static_cast<VkSamplerMipmapMode>(sampler_info->vk_mip_map_mode),
                    .addressModeU     = static_cast<VkSamplerAddressMode>(sampler_info->vk_address_mode_u),
                    .addressModeV     = static_cast<VkSamplerAddressMode>(sampler_info->vk_address_mode_v),
                    .addressModeW     = static_cast<VkSamplerAddressMode>(sampler_info->vk_address_mode_w),
                    .mipLodBias       = sampler_info->lod_bias,
                    .anisotropyEnable = VK_TRUE,
                    .maxAnisotropy    = sampler_info->max_anisotropy_clamp,
                    .compareEnable    = VK_TRUE,
                    .compareOp        = static_cast<VkCompareOp>(sampler_info->vk_compare_op),
                    .minLod           = sampler_info->min_lod_clamp,
                    .maxLod           = sampler_info->max_lod_clamp,
                    .borderColor      = static_cast<VkBorderColor>(sampler_info->vk_border_color)
                };
                
                const u32 result0 = ::vkCreateSampler(context->GetDevice(), std::addressof(vk_sampler_info), nullptr, std::addressof(m_vk_sampler));
                DD_ASSERT(result0 == VK_SUCCESS);
            }
            
            void Finalize(const Context *context) {
                ::vkDestroySampler(context->GetDevice(), m_vk_sampler, nullptr);
            }
            
            constexpr ALWAYS_INLINE VkSampler GetSampler() const { return m_vk_sampler; }
    };
}