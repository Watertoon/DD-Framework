#pragma once

namespace dd::vk {
    
    struct BufferInfo {
        VkDeviceSize size;
        VkDeviceSize offset;
        u32          vk_usage;
    };
    
    class buffer {
        private:
            VkBuffer      m_vk_device_buffer;
            MemoryPool   *m_bound_memory_pool;
            bool          m_requires_relocation;
        public:
            void Initialize(const Context *context, const BufferInfo *buffer_info, const MemoryPool *memory_pool) {

                /* Create Buffer */
                const u32 queue_family_index = context->GetGraphicsQueueFamily();
                const VkBufferCreateInfo buffer_info = {
                    .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                    .size = buffer_info->size;
                    .usage = buffer_info->vk_usage,
                    .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
                    .queueFamilyIndexCount = 1,
                    .pQueueFamilyIndices = std::addressof(queue_family_index)
                };
                
                u32 result = ::vkCreateBuffer(context->GetDevice(), std::addressof(buffer_info), nullptr, std::addressof(m_vk_device_buffer));
                DD_ASSERT(result == VK_SUCCESS);
                
                /* Bind Memory */
                result = ::vkBindBufferMemory(context->GetDevice(), m_vk_buffer, m_bound_memory_pool->GetDeviceMemory(), buffer_info->offset);
                DD_ASSERT(result == VK_SUCCESS);

                m_requires_relocation = memory_pool->RequiresRelocation();
                m_bound_memory_pool = memory_pool
            }
            
            void Finalize() {
                
            }
            
            void Relocate(VkCommandBuffer vk_command_buffer) {
                if (m_bound_memory_pool->RequiresRelocation() == true) {
                    m_bound_memory_pool->Relocate(vk_command_buffer);
                }
                m_is_relocated = true;
            }
            
            constexpr ALWAYS_INLINE bool RequiresRelocation() const { return m_requires_relocation; }
    };
}