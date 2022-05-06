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
    
    struct MemoryPoolInfo {
        VkDeviceSize size;
        u32          vk_memory_property_flags;
        void        *import_memory;
    };
    
    class MemoryPool {
        private:
            friend class Buffer;
        private:
            VkDeviceMemory  m_vk_host_memory;
            VkDeviceMemory  m_vk_device_memory;
            VkBuffer        m_vk_host_buffer;
            VkBuffer        m_vk_device_buffer;
            const Context  *m_parent_context;
            u32             m_vk_memory_property_flags;
            bool            m_requires_relocation;
            bool            m_is_device_memory;
            void           *m_host_pointer;
            VkDeviceSize    m_size;
            void           *m_mapped_memory;
            u32             m_map_count;
        public:
            constexpr MemoryPool() {/*...*/}
            
            void Initialize(const Context *context, const MemoryPoolInfo* pool_info) {
                DD_ASSERT(pool_info->import_memory != nullptr);
                const u32 pool_properties = pool_info->vk_memory_property_flags;
                
                m_size = pool_info->size;
                m_parent_context = context;
                m_vk_memory_property_flags = pool_properties;

                /* Check host pointer memory properties */
                VkMemoryHostPointerPropertiesEXT host_properties = { .sType = VK_STRUCTURE_TYPE_MEMORY_HOST_POINTER_PROPERTIES_EXT };
                const u32 result0 = pfn_vkGetMemoryHostPointerPropertiesEXT(context->GetDevice(), VK_EXTERNAL_MEMORY_HANDLE_TYPE_HOST_ALLOCATION_BIT_EXT, pool_info->import_memory, std::addressof(host_properties));
                DD_ASSERT(result0 != 0);

                /* Handle whether our host pointer needs a staging buffer */
                DD_ASSERT((host_properties.memoryTypeBits & (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) == VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
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

                    const VkMemoryAllocateInfo device_allocate_info =  {
                        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
                        .allocationSize = pool_info->size,
                        .memoryTypeIndex = device_memory_type
                    };
                    const u32 result1 = ::vkAllocateMemory(context->GetDevice(), std::addressof(device_allocate_info), nullptr, std::addressof(m_vk_device_memory));
                    DD_ASSERT(result1 == VK_SUCCESS);
                }
                DD_ASSERT(host_memory_type != -1);

                /* Create host memory */
                const VkImportMemoryHostPointerInfoEXT host_info = {
                    .sType = VK_STRUCTURE_TYPE_IMPORT_MEMORY_HOST_POINTER_INFO_EXT,
                    .handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_HOST_ALLOCATION_BIT_EXT,
                    .pHostPointer = pool_info->import_memory
                };
                const VkMemoryAllocateInfo host_allocate_info =  {
                    .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
                    .pNext = std::addressof(host_info),
                    .allocationSize = pool_info->size,
                    .memoryTypeIndex = host_memory_type
                };
                const u32 result2 = ::vkAllocateMemory(context->GetDevice(), std::addressof(host_allocate_info), nullptr, std::addressof(m_vk_host_memory));
                DD_ASSERT(result2 == VK_SUCCESS);
            }

            void Finalize(const Context *context) {
                const VkDevice device = context->GetDevice();
                if (m_vk_host_buffer != 0) {
                    ::vkDestroyBuffer(device, m_vk_host_buffer, nullptr);
                    m_vk_host_buffer = 0;
                }
                if (m_vk_host_memory != 0) {
                    ::vkFreeMemory(device, m_vk_host_memory, nullptr);
                    m_vk_host_memory = 0;
                }
                if (m_vk_device_buffer != 0) {
                    ::vkDestroyBuffer(device, m_vk_device_buffer, nullptr);
                    m_vk_device_buffer = 0;
                }
                if (m_vk_device_memory != 0) {
                    ::vkFreeMemory(device, m_vk_device_memory, nullptr);
                    m_vk_device_memory = 0;
                }
            }
            
            void *Map() {
                if (m_mapped_memory == nullptr) {
                    const u32 result0 = ::vkMapMemory(GetGlobalContext()->GetDevice(), this->GetDeviceMemory(), 0, VK_WHOLE_SIZE, 0, std::addressof(m_mapped_memory));
                    DD_ASSERT(result0 == VK_SUCCESS);
                }
                m_map_count += 1;
                return m_mapped_memory;
            }
            
            void Unmap() {
                m_map_count -= 1;
                if (m_map_count == 0) {
                    ::vkUnmapMemory(GetGlobalContext()->GetDevice(), this->GetDeviceMemory());
                    m_mapped_memory = nullptr;
                }
            }

            constexpr VkDeviceMemory GetDeviceMemory() const { return (m_is_device_memory == false) ? m_vk_host_memory : m_vk_device_memory; }
            
            constexpr bool RequiresRelocation() const { return m_requires_relocation; }

            void Relocate(VkCommandBuffer vk_command_buffer) {
                /* Copy entire memory pool to buffer without host properties */
                if (m_requires_relocation == true) {
                    /* Create staging buffers spanning host and device memory pools */
                    const u32 queue_family_index = m_parent_context->GetGraphicsQueueFamilyIndex();
                    const VkBufferCreateInfo src_buffer_info = {
                        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                        .size = m_size,
                        .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
                        .queueFamilyIndexCount = 1,
                        .pQueueFamilyIndices = std::addressof(queue_family_index)
                    };

                    const u32 result0 = ::vkCreateBuffer(m_parent_context->GetDevice(), std::addressof(src_buffer_info), nullptr, std::addressof(m_vk_host_buffer));
                    DD_ASSERT(result0 == VK_SUCCESS);

                    const u32 result1 = ::vkBindBufferMemory(m_parent_context->GetDevice(), m_vk_host_buffer, m_vk_host_memory, 0);
                    DD_ASSERT(result1 == VK_SUCCESS);

                    const VkBufferCreateInfo dst_buffer_info = {
                        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                        .size = m_size,
                        .usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
                        .queueFamilyIndexCount = 1,
                        .pQueueFamilyIndices = std::addressof(queue_family_index)
                    };
                    const u32 result2 = ::vkCreateBuffer(m_parent_context->GetDevice(), std::addressof(dst_buffer_info), nullptr, std::addressof(m_vk_device_buffer));
                    DD_ASSERT(result2 == VK_SUCCESS);

                    const u32 result3 = ::vkBindBufferMemory(m_parent_context->GetDevice(), m_vk_device_buffer, m_vk_device_memory, 0);
                    DD_ASSERT(result3 == VK_SUCCESS);

                    /* Copy memory to device local buffer */
                    const VkBufferCopy copy_info = {
                        .size = m_size
                    };
                    ::vkCmdCopyBuffer(vk_command_buffer, m_vk_host_buffer, m_vk_device_buffer, 1, std::addressof(copy_info));
                    m_requires_relocation = false;
                }
            }
            
            static constexpr u64 GetAlignment(const Context *context) {
                return context->GetPhysicalDeviceProperties()->properties.limits.minMemoryMapAlignment;
            }
    };
}