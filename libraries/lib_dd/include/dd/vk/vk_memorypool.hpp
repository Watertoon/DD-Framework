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

    struct MemoryPoolInfo {
        VkDeviceSize size;
        u32          vk_memory_property_flags;
        void        *import_memory;
    };

    class MemoryPool {
        private:
            friend class Buffer;
            friend class Texture;
        private:
            VkDeviceMemory  m_vk_memory;
            VkDeviceMemory  m_vk_staging_memory;
            VkBuffer        m_vk_staging_host_buffer;
            VkBuffer        m_vk_staging_device_buffer;
            u32             m_vk_memory_property_flags;
            bool            m_requires_relocation;
            bool            m_is_device_memory;
            bool            m_is_import;
            void           *m_host_pointer;
            VkDeviceSize    m_size;
            void           *m_mapped_memory;
        public:
            void RelocateHostMemoryToDevice(VkCommandBuffer vk_command_buffer) {

                /* Do nothing if we are not device memory */
                if (m_requires_relocation == false) { return; }

                /* Create host staging buffer */
                const u32 queue_family_index = GetGlobalContext()->GetGraphicsQueueFamilyIndex();
                {
                    const VkExternalMemoryBufferCreateInfo external_info = {
                        .sType = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_BUFFER_CREATE_INFO,
                        .handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_HOST_ALLOCATION_BIT_EXT
                    };

                    const VkBufferCreateInfo src_buffer_info = {
                        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                        .pNext = std::addressof(external_info),
                        .size = m_size,
                        .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
                        .queueFamilyIndexCount = 1,
                        .pQueueFamilyIndices = std::addressof(queue_family_index)
                    };

                    const u32 result0 = ::pfn_vkCreateBuffer(GetGlobalContext()->GetDevice(), std::addressof(src_buffer_info), nullptr, std::addressof(m_vk_staging_host_buffer));
                    DD_ASSERT(result0 == VK_SUCCESS);
                    
                    const u32 result1 = ::pfn_vkBindBufferMemory(GetGlobalContext()->GetDevice(), m_vk_staging_host_buffer, m_vk_staging_memory, 0);
                    DD_ASSERT(result1 == VK_SUCCESS);
                }

                /* Create device staging buffer */
                {
                    const VkBufferCreateInfo dst_buffer_info = {
                        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                        .size = m_size,
                        .usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
                        .queueFamilyIndexCount = 1,
                        .pQueueFamilyIndices = std::addressof(queue_family_index)
                    };
                    const u32 result2 = ::pfn_vkCreateBuffer(GetGlobalContext()->GetDevice(), std::addressof(dst_buffer_info), nullptr, std::addressof(m_vk_staging_device_buffer));
                    DD_ASSERT(result2 == VK_SUCCESS);

                    const u32 result3 = ::pfn_vkBindBufferMemory(GetGlobalContext()->GetDevice(), m_vk_staging_device_buffer, m_vk_memory, 0);
                    DD_ASSERT(result3 == VK_SUCCESS);
                }

                /* Copy memory to device local buffer */
                const VkBufferCopy copy_info = {
                    .size = m_size
                };
                ::pfn_vkCmdCopyBuffer(vk_command_buffer, m_vk_staging_host_buffer, m_vk_staging_device_buffer, 1, std::addressof(copy_info));

                /* Barrier copy operation */
                const VkBufferMemoryBarrier2 buffer_barrier = {
                    .sType         = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
                    .dstStageMask  = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
                    .dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
                    .buffer        = m_vk_staging_device_buffer,
                    .size          = VK_WHOLE_SIZE
                };

                const VkDependencyInfo dependency_info = {
                    .sType                    = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
                    .dependencyFlags          = VK_DEPENDENCY_BY_REGION_BIT,
                    .bufferMemoryBarrierCount = 1,
                    .pBufferMemoryBarriers    = std::addressof(buffer_barrier)
                };

                ::pfn_vkCmdPipelineBarrier2(vk_command_buffer, std::addressof(dependency_info));

                m_requires_relocation = false;
            }
        public:
            constexpr MemoryPool() {/*...*/}
            
            void Initialize(const Context *context, const MemoryPoolInfo* pool_info) {
                DD_ASSERT(pool_info->import_memory != nullptr);

                const u32 pool_properties = pool_info->vk_memory_property_flags;

                /* Set size and flags */
                m_size = pool_info->size;
                m_vk_memory_property_flags = pool_properties;

                /* Check host pointer memory properties */
                VkMemoryHostPointerPropertiesEXT host_properties = { 
                    .sType = VK_STRUCTURE_TYPE_MEMORY_HOST_POINTER_PROPERTIES_EXT
                };
                const u32 result0 = ::pfn_vkGetMemoryHostPointerPropertiesEXT(context->GetDevice(), VK_EXTERNAL_MEMORY_HANDLE_TYPE_HOST_ALLOCATION_BIT_EXT, pool_info->import_memory, std::addressof(host_properties));
                DD_ASSERT(result0 == VK_SUCCESS);

                /* Handle whether our host pointer needs a staging buffer */
                s32 host_memory_type = 0;
                if (((host_properties.memoryTypeBits & pool_properties) == pool_properties)) {
                    host_memory_type = context->FindMemoryHeapIndex(pool_properties);
                    m_requires_relocation = false;
                    m_is_device_memory = false;
                } else {
                    host_memory_type = context->FindMemoryHeapIndex(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
                    m_requires_relocation = true;
                    m_is_device_memory = true;

                    /* Allocate our device memory */
                    const s32 device_memory_type = context->FindMemoryHeapIndex(pool_properties);
                    DD_ASSERT(device_memory_type != -1);

                    const VkMemoryAllocateFlagsInfo device_allocate_flags = {
                        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO,
                        .flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT
                    };
                    const VkMemoryAllocateInfo device_allocate_info =  {
                        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
                        .pNext = std::addressof(device_allocate_flags),
                        .allocationSize = pool_info->size,
                        .memoryTypeIndex = static_cast<u32>(device_memory_type)
                    };
                    const u32 result1 = ::pfn_vkAllocateMemory(context->GetDevice(), std::addressof(device_allocate_info), nullptr, std::addressof(m_vk_memory));
                    DD_ASSERT(result1 == VK_SUCCESS);
                }
                DD_ASSERT(host_memory_type != -1);

                /* Create host memory */
                const VkMemoryAllocateFlagsInfo host_allocate_flags = {
                    .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO,
                    .flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT
                };
                const VkImportMemoryHostPointerInfoEXT host_info = {
                    .sType = VK_STRUCTURE_TYPE_IMPORT_MEMORY_HOST_POINTER_INFO_EXT,
                    .pNext = std::addressof(host_allocate_flags),
                    .handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_HOST_ALLOCATION_BIT_EXT,
                    .pHostPointer = pool_info->import_memory
                };
                const VkMemoryAllocateInfo host_allocate_info =  {
                    .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
                    .pNext = std::addressof(host_info),
                    .allocationSize = pool_info->size,
                    .memoryTypeIndex = static_cast<u32>(host_memory_type)
                };
                const u32 result2 = ::pfn_vkAllocateMemory(context->GetDevice(), std::addressof(host_allocate_info), nullptr, (m_requires_relocation) ? std::addressof(m_vk_staging_memory) : std::addressof(m_vk_memory));
                DD_ASSERT(result2 == VK_SUCCESS);
            }

            void ImportExisting(VkDeviceMemory import_memory, bool is_device_local) {
                m_vk_memory = import_memory;
                m_is_device_memory = is_device_local;
                m_is_import = true;
            }

            void Finalize(const Context *context) {

                if (m_is_import == true) {
                    return;
                }

                const VkDevice device = context->GetDevice();
                if (m_vk_staging_device_buffer != 0) {
                    ::pfn_vkDestroyBuffer(device, m_vk_staging_device_buffer, nullptr);
                    m_vk_staging_device_buffer = 0;
                }
                if (m_vk_memory != 0) {
                    ::pfn_vkFreeMemory(device, m_vk_memory, nullptr);
                    m_vk_memory = 0;
                }
                if (m_vk_staging_host_buffer != 0) {
                    ::pfn_vkDestroyBuffer(device, m_vk_staging_host_buffer, nullptr);
                    m_vk_staging_device_buffer = 0;
                }
                if (m_vk_staging_memory != 0) {
                    ::pfn_vkFreeMemory(device, m_vk_staging_memory, nullptr);
                    m_vk_staging_memory = 0;
                }
            }

            void *Map() {
                if (m_mapped_memory == nullptr) {
                    const u32 result0 = ::pfn_vkMapMemory(GetGlobalContext()->GetDevice(), m_vk_memory, 0, VK_WHOLE_SIZE, 0, std::addressof(m_mapped_memory));
                    DD_ASSERT(result0 == VK_SUCCESS);
                }
                return m_mapped_memory;
            }

            void Unmap() {
                return;
            }

            constexpr VkDeviceMemory GetDeviceMemory() const { return m_vk_memory; }

            constexpr bool RequiresRelocation() const { return m_requires_relocation; }

            constexpr bool IsHostMemory() const { return (m_is_device_memory == false); }

            static constexpr u64 GetAlignment(const Context *context) {
                return context->GetPhysicalDeviceProperties()->properties.limits.minMemoryMapAlignment;
            }
    };
}
