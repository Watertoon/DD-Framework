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

    struct TextureInfo {
        u32 vk_create_flags;
        u16 vk_usage_flags;
        u16 mip_levels;
        u32 width;
        u32 height;
        u32 depth;
        u32 vk_format;
        u32 vk_image_layout;
        u8  vk_image_type;
        u8  vk_sample_count_flag;
        u8  vk_tiling;
        u8  array_layers;
        u32 memory_offset;
    };

    class Texture {
        private:
            VkImage     m_vk_image;
            MemoryPool *m_bound_memory_pool;
            u32         m_vk_image_layout;
            bool        m_import;
            bool        m_requires_relocation;
        public:
            constexpr Texture() {/*...*/}

            void Initialize(const Context *context, const TextureInfo *texture_info, MemoryPool *memory_pool) {

                /* Create Image */
                const u32 queue_family_index = context->GetGraphicsQueueFamilyIndex();
                const VkImageCreateInfo image_info {
                    .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
                    .flags = texture_info->vk_create_flags,
                    .imageType = static_cast<VkImageType>(texture_info->vk_image_type),
                    .format = static_cast<VkFormat>(texture_info->vk_format),
                    .extent = {
                        .width = texture_info->width,
                        .height = texture_info->height,
                        .depth = texture_info->depth
                    },
                    .mipLevels = texture_info->mip_levels,
                    .arrayLayers = texture_info->array_layers,
                    .samples = static_cast<VkSampleCountFlagBits>(texture_info->vk_sample_count_flag),
                    .tiling = static_cast<VkImageTiling>(texture_info->vk_tiling),
                    .usage = texture_info->vk_usage_flags,
                    .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
                    .queueFamilyIndexCount = 1,
                    .pQueueFamilyIndices = std::addressof(queue_family_index),
                    .initialLayout = static_cast<VkImageLayout>(texture_info->vk_image_layout)
                };

                const u32 result0 = ::pfn_vkCreateImage(context->GetDevice(), std::addressof(image_info), nullptr, std::addressof(m_vk_image));
                DD_ASSERT(result0 == VK_SUCCESS);

                /* Bind image memory */
                const u32 result1 = ::pfn_vkBindImageMemory(context->GetDevice(), m_vk_image, memory_pool->GetDeviceMemory(), texture_info->memory_offset);
                DD_ASSERT(result1 == VK_SUCCESS);

                m_requires_relocation = memory_pool->RequiresRelocation();
                m_bound_memory_pool = memory_pool;

                m_vk_image_layout = texture_info->vk_image_layout;
            }

            void ImportExisting(VkImage image) {
                m_vk_image = image;
                m_import = true;
            }

            void Finalize(const Context *context) {
                if (m_import == false) {
                    ::pfn_vkDestroyImage(context->GetDevice(), m_vk_image, nullptr);
                }
            }

            void Relocate(VkCommandBuffer vk_command_buffer) {
                if (m_bound_memory_pool->RequiresRelocation() == true) {
                    m_bound_memory_pool->RelocateHostMemoryToDevice(vk_command_buffer);
                }
                m_requires_relocation = true;
            }

            constexpr ALWAYS_INLINE bool RequiresRelocation() const { return m_requires_relocation; }

            constexpr ALWAYS_INLINE VkImage GetImage() const { return m_vk_image; }

            constexpr ALWAYS_INLINE VkImageLayout GetImageLayout() const { return static_cast<VkImageLayout>(m_vk_image_layout); }

            void SetImageLayout(VkImageLayout new_layout) { m_vk_image_layout = static_cast<u32>(new_layout); }

            static u64 GetAlignment(const Context *context, const TextureInfo *texture_info) { 

                /* Create staging Image */
                const u32 queue_family_index = context->GetGraphicsQueueFamilyIndex();
                const VkImageCreateInfo image_info {
                    .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
                    .flags = texture_info->vk_create_flags,
                    .imageType = static_cast<VkImageType>(texture_info->vk_image_type),
                    .format = static_cast<VkFormat>(texture_info->vk_format),
                    .extent = {
                        .width = texture_info->width,
                        .height = texture_info->height,
                        .depth = texture_info->depth
                    },
                    .mipLevels = texture_info->mip_levels,
                    .arrayLayers = texture_info->array_layers,
                    .samples = static_cast<VkSampleCountFlagBits>(texture_info->vk_sample_count_flag),
                    .tiling = static_cast<VkImageTiling>(texture_info->vk_tiling),
                    .usage = texture_info->vk_usage_flags,
                    .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
                    .queueFamilyIndexCount = 1,
                    .pQueueFamilyIndices = std::addressof(queue_family_index),
                    .initialLayout = static_cast<VkImageLayout>(texture_info->vk_image_layout)
                };

                const VkDeviceImageMemoryRequirements requirements_info = {
                    .sType = VK_STRUCTURE_TYPE_DEVICE_IMAGE_MEMORY_REQUIREMENTS,
                    .pCreateInfo = std::addressof(image_info),
                    .planeAspect = VK_IMAGE_ASPECT_COLOR_BIT
                };

                VkMemoryRequirements2 memory_requirements = {
                    .sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2
                };

                ::pfn_vkGetDeviceImageMemoryRequirements(context->GetDevice(), std::addressof(requirements_info), std::addressof(memory_requirements));
                
                return memory_requirements.memoryRequirements.alignment;
            }

            static u64 GetRequiredMemory(const Context *context, const TextureInfo *texture_info) { 

                /* Create staging Image */
                const u32 queue_family_index = context->GetGraphicsQueueFamilyIndex();
                const VkImageCreateInfo image_info {
                    .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
                    .flags = texture_info->vk_create_flags,
                    .imageType = static_cast<VkImageType>(texture_info->vk_image_type),
                    .format = static_cast<VkFormat>(texture_info->vk_format),
                    .extent = {
                        .width = texture_info->width,
                        .height = texture_info->height,
                        .depth = texture_info->depth
                    },
                    .mipLevels = texture_info->mip_levels,
                    .arrayLayers = texture_info->array_layers,
                    .samples = static_cast<VkSampleCountFlagBits>(texture_info->vk_sample_count_flag),
                    .tiling = static_cast<VkImageTiling>(texture_info->vk_tiling),
                    .usage = texture_info->vk_usage_flags,
                    .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
                    .queueFamilyIndexCount = 1,
                    .pQueueFamilyIndices = std::addressof(queue_family_index),
                    .initialLayout = static_cast<VkImageLayout>(texture_info->vk_image_layout)
                };

                const VkDeviceImageMemoryRequirements requirements_info = {
                    .sType = VK_STRUCTURE_TYPE_DEVICE_IMAGE_MEMORY_REQUIREMENTS,
                    .pCreateInfo = std::addressof(image_info),
                    .planeAspect = VK_IMAGE_ASPECT_COLOR_BIT
                };

                VkMemoryRequirements2 memory_requirements = {
                    .sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2
                };

                ::pfn_vkGetDeviceImageMemoryRequirements(context->GetDevice(), std::addressof(requirements_info), std::addressof(memory_requirements));
                
                return memory_requirements.memoryRequirements.size;
            }
    };
}
