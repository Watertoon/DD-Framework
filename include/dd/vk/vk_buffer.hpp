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

    struct BufferInfo {
        VkDeviceSize size;
        VkDeviceSize offset;
        u32          vk_usage;
    };

    class Buffer {
        private:
            MemoryPool   *m_bound_memory_pool;
            VkBuffer      m_vk_buffer;
            size_t        m_memory_offset;
            bool          m_requires_relocation;
        public:
            constexpr Buffer() {/*...*/}

            void Initialize(const Context *context, const BufferInfo *buffer_info, MemoryPool *memory_pool) {

                /* Create buffer */
                const VkExternalMemoryBufferCreateInfo external_info = {
                    .sType = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_BUFFER_CREATE_INFO,
                    .handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_HOST_ALLOCATION_BIT_EXT
                };

                const u32 queue_family_index = context->GetGraphicsQueueFamilyIndex();
                const VkBufferCreateInfo buffer_create_info = {
                    .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                    .pNext = (memory_pool->IsHostMemory() == true) ? std::addressof(external_info) : nullptr,
                    .size = buffer_info->size,
                    .usage = buffer_info->vk_usage | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT_KHR,
                    .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
                    .queueFamilyIndexCount = 1,
                    .pQueueFamilyIndices = std::addressof(queue_family_index)
                };

                const u32 result0 = ::pfn_vkCreateBuffer(context->GetDevice(), std::addressof(buffer_create_info), nullptr, std::addressof(m_vk_buffer));
                DD_ASSERT(result0 == VK_SUCCESS);

                /* Bind buffer memory */
                const u32 result1 = ::pfn_vkBindBufferMemory(context->GetDevice(), m_vk_buffer, memory_pool->GetDeviceMemory(), buffer_info->offset);
                DD_ASSERT(result1 == VK_SUCCESS);

                m_requires_relocation = memory_pool->RequiresRelocation();
                m_bound_memory_pool = memory_pool;
                m_memory_offset = buffer_info->offset;
            }

            void Finalize(const Context *context) {
                ::pfn_vkDestroyBuffer(context->GetDevice(), m_vk_buffer, nullptr);
                m_vk_buffer = 0;
            }

            void *Map() {
                uintptr_t address = reinterpret_cast<uintptr_t>(m_bound_memory_pool->Map()) + m_memory_offset;
                return reinterpret_cast<void*>(address);
            }

            void Unmap() {
                m_bound_memory_pool->Unmap();
            }
            
            VkDeviceAddress GetGpuAddress() {
                const VkBufferDeviceAddressInfo device_address_info = {
                    .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
                    .buffer = m_vk_buffer
                };
                return ::pfn_vkGetBufferDeviceAddress(GetGlobalContext()->GetDevice(), std::addressof(device_address_info));
            }
            
            constexpr ALWAYS_INLINE VkBuffer GetBuffer() const { return m_vk_buffer; }

            void Relocate(VkCommandBuffer vk_command_buffer) {
                if (m_bound_memory_pool->RequiresRelocation() == true) {
                    m_bound_memory_pool->Relocate(vk_command_buffer);
                }
                m_requires_relocation = true;
            }

            constexpr ALWAYS_INLINE bool RequiresRelocation() const { return m_requires_relocation; }

            static u64 GetAlignment(const Context *context, const BufferInfo *buffer_info) {

                /* Use staging buffer to query alignment */
                VkBuffer vk_buffer;
                const u32 queue_family_index = context->GetGraphicsQueueFamilyIndex();
                const VkBufferCreateInfo buffer_create_info = {
                    .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                    .size = buffer_info->size,
                    .usage = buffer_info->vk_usage,
                    .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
                    .queueFamilyIndexCount = 1,
                    .pQueueFamilyIndices = std::addressof(queue_family_index)
                };

                const u32 result0 = ::pfn_vkCreateBuffer(context->GetDevice(), std::addressof(buffer_create_info), nullptr, std::addressof(vk_buffer));
                DD_ASSERT(result0 == VK_SUCCESS);

                VkMemoryRequirements memory_requirements = {};
                ::pfn_vkGetBufferMemoryRequirements(context->GetDevice(), vk_buffer, std::addressof(memory_requirements));

                ::pfn_vkDestroyBuffer(context->GetDevice(), vk_buffer, nullptr);
                
                return memory_requirements.alignment;
            }
    };
}
