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
            bool          m_requires_relocation;
        public:
            constexpr Buffer() {/*...*/}

            void Initialize(const Context *context, const BufferInfo *buffer_info, MemoryPool *memory_pool) {

                /* Create buffer */
                const u32 queue_family_index = context->GetGraphicsQueueFamilyIndex();
                const VkBufferCreateInfo buffer_create_info = {
                    .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                    .size = buffer_info->size,
                    .usage = buffer_info->vk_usage,
                    .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
                    .queueFamilyIndexCount = 1,
                    .pQueueFamilyIndices = std::addressof(queue_family_index)
                };

                const u32 result0 = ::vkCreateBuffer(context->GetDevice(), std::addressof(buffer_create_info), nullptr, std::addressof(m_vk_buffer));
                DD_ASSERT(result0 == VK_SUCCESS);

                /* Bind buffer memory */
                const u32 result1 = ::vkBindBufferMemory(context->GetDevice(), m_vk_buffer, memory_pool->GetDeviceMemory(), buffer_info->offset);
                DD_ASSERT(result1 == VK_SUCCESS);

                m_requires_relocation = memory_pool->RequiresRelocation();
                m_bound_memory_pool = memory_pool;
            }

            void Finalize(const Context *context) {
                ::vkDestroyBuffer(context->GetDevice(), m_vk_buffer, nullptr);
                m_vk_buffer = 0;
            }

            void Relocate(VkCommandBuffer vk_command_buffer) {
                if (m_bound_memory_pool->RequiresRelocation() == true) {
                    m_bound_memory_pool->Relocate(vk_command_buffer);
                }
                m_requires_relocation = true;
            }

            constexpr ALWAYS_INLINE bool RequiresRelocation() const { return m_requires_relocation; }

            constexpr ALWAYS_INLINE VkBuffer GetBuffer() const { return m_vk_buffer; }
    };
}