#pragma once

namespace dd::vk {
    
    typedef u32 DescriptorSlot;
    
    struct GpuAddress {
        VkDeviceAddress address;
    };

    class DescriptorPool {
        public:
            static constexpr size_t MaxTextureSlots = Context::TargetMaxTextureDescriptors;
            static constexpr size_t MaxSamplerSlots = Context::TargetMaxSamplerDescriptors;
            static constexpr size_t FreeListDefaultSize = 64;
        private:
            VkDescriptorPool  m_vk_descriptor_pool;
            VkDescriptorSet   m_vk_descriptor_set;
            u32               m_vk_pool_type;
            u32               m_max_slot_ids;
            u32               m_texture_id_count;
            u32               m_free_id_stack_size;
            u32               m_free_id_stack_count;
            u32              *m_free_id_stack;
        private:
            u32 GetNewTextureId() {
                if (m_free_id_stack_count != 0) {
                    m_free_id_stack_count -= 1;
                    return m_free_id_stack[m_free_id_stack_count];
                }
                DD_ASSERT(m_texture_id_count != m_max_slot_ids);

                return m_texture_id_count++;
            }
        public:
            constexpr DescriptorPool() {}
            
            void Initialize(const Context *context, VkDescriptorType descriptor_type, u32 max_descriptor_slots) {
                DD_ASSERT(descriptor_type == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE || descriptor_type == VK_DESCRIPTOR_TYPE_SAMPLER);

                /* Create Descriptor Pool */
                const VkDescriptorPoolSize pool_size = {
                    .type = descriptor_type,
                    .descriptorCount = max_descriptor_slots
                };

                const VkDescriptorPoolCreateInfo pool_info = {
                    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
                    .flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT,
                    .maxSets = 1,
                    .poolSizeCount = 1,
                    .pPoolSizes = std::addressof(pool_size)
                };

                const u32 result0 = ::vkCreateDescriptorPool(context->GetDevice(), std::addressof(pool_info), nullptr, std::addressof(m_vk_descriptor_pool));
                DD_ASSERT(result0 == VK_SUCCESS);

                /* Create Descriptor Set */
                VkDescriptorSetLayout vk_descriptor_set_layout;
                if (descriptor_type == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE) {
                    vk_descriptor_set_layout = context->GetTextureDescriptorSetLayout();
                } else if (descriptor_type == VK_DESCRIPTOR_TYPE_SAMPLER) {
                    vk_descriptor_set_layout = context->GetSamplerDescriptorSetLayout();
                }

                const VkDescriptorSetAllocateInfo set_info = {
                    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
                    .descriptorPool = m_vk_descriptor_pool,
                    .descriptorSetCount = 1,
                    .pSetLayouts = std::addressof(vk_descriptor_set_layout)
                };

                const u32 result1 = ::vkAllocateDescriptorSets(context->GetDevice(), std::addressof(set_info), std::addressof(m_vk_descriptor_set));
                DD_ASSERT(result1 == VK_SUCCESS);

                /* Initalize default free list size */
                m_free_id_stack = new u32[FreeListDefaultSize];
                DD_ASSERT(m_free_id_stack != nullptr);

                m_free_id_stack_size = FreeListDefaultSize;
                
                m_vk_pool_type = descriptor_type;
                m_max_slot_ids = max_descriptor_slots;
                m_texture_id_count = 0;
            }

            void Finalize(const Context *context) {
                ::vkDestroyDescriptorPool(context->GetDevice(), m_vk_descriptor_pool, nullptr);
            }

            DescriptorSlot RegisterTexture(const TextureView *texture_view) {
                DD_ASSERT(m_vk_pool_type == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);

                DescriptorSlot new_id = this->GetNewTextureId();

                const VkDescriptorImageInfo image_info = {
                    .imageView   = texture_view->GetImageView(),
                    .imageLayout = texture_view->GetTexture()->GetImageLayout()
                };
                const VkWriteDescriptorSet write_set = {
                    .sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                    .dstSet          = m_vk_descriptor_set,
                    .dstBinding      = Context::TargetSamplerDescriptorBinding,
                    .dstArrayElement = new_id,
                    .descriptorCount = 1,
                    .descriptorType  = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                    .pImageInfo      = std::addressof(image_info)
                };

                ::vkUpdateDescriptorSets(GetGlobalContext()->GetDevice(), 1, std::addressof(write_set), 0, nullptr);

                return new_id;
            }

            DescriptorSlot RegisterSampler(const Sampler *sampler) {
                DD_ASSERT(m_vk_pool_type == VK_DESCRIPTOR_TYPE_SAMPLER);

                DescriptorSlot new_id = this->GetNewTextureId();

                const VkDescriptorImageInfo image_info = {
                    .sampler = sampler->GetSampler()
                };
                const VkWriteDescriptorSet write_set = {
                    .sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                    .dstSet          = m_vk_descriptor_set,
                    .dstBinding      = Context::TargetSamplerDescriptorBinding,
                    .dstArrayElement = new_id,
                    .descriptorCount = 1,
                    .descriptorType  = VK_DESCRIPTOR_TYPE_SAMPLER,
                    .pImageInfo      = std::addressof(image_info)
                };

                ::vkUpdateDescriptorSets(GetGlobalContext()->GetDevice(), 1, std::addressof(write_set), 0, nullptr);

                return new_id;
            }

            void UnregisterResourceBySlot(DescriptorSlot slot) {
                /* Resize Free List if necessary */
                if (m_free_id_stack_size == m_free_id_stack_count) {

                    m_free_id_stack_size = m_free_id_stack_size * 2;
                    u32 *new_stack = new u32[m_free_id_stack_size];
                    DD_ASSERT(new_stack != nullptr);

                    ::memcpy(new_stack, m_free_id_stack, m_free_id_stack_count * sizeof(u32));

                    delete[] m_free_id_stack;
                    m_free_id_stack = new_stack;
                }

                /* Add new freed slot */
                m_free_id_stack[m_free_id_stack_count] = slot;
                m_free_id_stack_size += 1;
            }

            constexpr VkDescriptorSet GetDescriptorSet() const { return m_vk_descriptor_set; }

            constexpr VkDescriptorPool GetDescriptorPool() const { return m_vk_descriptor_pool; }

            constexpr u32 GetPoolType() const { return m_vk_pool_type; }
    };
}
