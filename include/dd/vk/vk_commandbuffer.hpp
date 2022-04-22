#pragma once

namespace dd::vk {

    class CommandBuffer {
        private:
            friend class Context;
        private:
            bool            m_is_convert;
            bool            m_is_rendering;
            VkCommandBuffer m_vk_command_buffer;
        public:
            constexpr CommandBuffer(VkCommandBuffer external_handle) : is_convert(false), m_vk_command_buffer(0) { /*...*/ }

            void Initialize(const Context& vk_context) {
                const VkCommandBufferAllocateInfo allocate_info = {
                    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                    .commandPool = vk_context.GetGraphicsCommandPool(),
                    .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                    .commandBufferCount = 1
                };

                const u32 result = ::vkAllocateCommandBuffers(vk_context.GetDevice(), std::addressof(allocate_info), std::addressof(m_vk_command_buffer));
                DD_ASSERT(result == VK_SUCCESS);
            }

            void Finalize(const Context& vk_context) {
                ::vkFreeCommandBuffers(vk_context.GetDevice(), vk_context.GetGraphicsCommandPool(), 1, std::addressof(m_vk_command_buffer));
            }

            void Begin() {
                const VkCommandBufferBeginInfo begin_info = {
                    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                    .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
                };
                
                const u32 result = ::vkBeginCommandBuffer(m_vk_command_buffer, std::addressof(begin_info));
                DD_ASSERT(result == VK_SUCCESS)
            }

            void End() {
                if (m_is_rendering == true) {
                    m_is_rendering = false;
                    ::vkCmdEndRendering(m_vk_command_buffer);
                }

                const u32 result = ::vkEndCommandBuffer(m_vk_command_buffer);
                DD_ASSERT(result == VK_SUCCESS);
            }
            
            void BindIndexBuffer(Buffer *buffer) {
                /* Relocate memory pool to device memory if required */
                if (buffer->RequiresRelocation() == true) {
                    buffer->RelocateBuffer(m_vk_command_buffer);
                }
                
                /* Bind Buffer */
                ::vkCmdBindIndexBuffer();
            }

            void ClearColorTarget(ColorTarget *color_target, const VkClearColor& color) {
                if (m_is_rendering == true) {
                    m_is_rendering = false;
                    ::vkCmdEndRendering(m_vk_command_buffer);
                }

                VkImageSubresourceRange sub_range = {};
                color->BuildSubresourceRange(std::addressof(sub_range));

                ::vkCmdClearColorImage(m_vk_command_buffer, color_target->GetImage(), color_target->GetImageLayout(), std::addressof(color), 1, std::addressof(sub_range));
            }

            void ClearColorTarget(ColorTarget *depth_stencil_target, const VkClearDepthStencilValue clear_value) {
                if (m_is_rendering == true) {
                    m_is_rendering = false;
                    ::vkCmdEndRendering(m_vk_command_buffer);
                }

                VkImageSubresourceRange sub_range = {};
                depth_stencil_target->BuildSubresourceRange(std::addressof(sub_range));

                ::vkCmdClearDepthStencilImage(m_vk_command_buffer, depth_stencil_target->GetImage(), depth_stencil_target->GetImageLayout(), std::addressof(clear_value), 1, std::addressof(sub_range));
            }

            void Draw();
            
            void SetPipeline(Pipeline *pipeline) {
                ::vkCmdBindPipeline();
            }

            void SetRenderTargets(s32 color_target_count, const ColorTarget **color_targets, const DepthStencilTarget *depth_stencil_target) {
                /* Only 8 color attachments are supported at time */
                DD_ASSERT(0 < color_target_count && color_target_count <= 8);

                /* Start a new render pass instance if one already exists */
                if (m_is_rendering == true) {
                    m_is_rendering = false;
                    ::vkCmdEndRendering(m_vk_command_buffer);
                }

                /* Initialize our color target attachments */
                const VkRenderingAttachmentInfo color_attachments[8] = {
                    {
                        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                        .imageView = (0 < color_target_count) ? color_attachments[0]->GetImageView() : nullptr,
                        .imageLayout = (0 < color_target_count) ? color_attachments[0]->GetImageLayout() : nullptr,
                        .resolveMode = VK_RESOLVE_MODE_NONE,
                        .resolveImageView = 0,
                        .resolveImageLayout = 0,
                        .loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                        .storeOp = VK_ATTACHMENT_STORE_OP_STORE
                    },
                    {
                        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                        .imageView = (1 < color_target_count) ? color_attachments[1]->GetImageView() : nullptr,
                        .imageLayout = (1 < color_target_count) ? color_attachments[1]->GetImageLayout() : nullptr,
                        .resolveMode = VK_RESOLVE_MODE_NONE,
                        .resolveImageView = 0,
                        .resolveImageLayout = 0,
                        .loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                        .storeOp = VK_ATTACHMENT_STORE_OP_STORE
                    },
                    {
                        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                        .imageView = (2 < color_target_count) ? color_attachments[2]->GetImageView() : nullptr,
                        .imageLayout = (2 < color_target_count) ? color_attachments[2]->GetImageLayout() : nullptr,
                        .resolveMode = VK_RESOLVE_MODE_NONE,
                        .resolveImageView = 0,
                        .resolveImageLayout = 0,
                        .loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                        .storeOp = VK_ATTACHMENT_STORE_OP_STORE
                    },
                    {
                        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                        .imageView = (3 < color_target_count) ? color_attachments[3]->GetImageView() : nullptr,
                        .imageLayout = (3 < color_target_count) ? color_attachments[3]->GetImageLayout() : nullptr,
                        .resolveMode = VK_RESOLVE_MODE_NONE,
                        .resolveImageView = 0,
                        .resolveImageLayout = 0,
                        .loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                        .storeOp = VK_ATTACHMENT_STORE_OP_STORE
                    },
                    {
                        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                        .imageView = (4 < color_target_count) ? color_attachments[4]->GetImageView() : nullptr,
                        .imageLayout = (4 < color_target_count) ? color_attachments[4]->GetImageLayout() : nullptr,
                        .resolveMode = VK_RESOLVE_MODE_NONE,
                        .resolveImageView = 0,
                        .resolveImageLayout = 0,
                        .loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                        .storeOp = VK_ATTACHMENT_STORE_OP_STORE
                    },
                    {
                        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                        .imageView = (5 < color_target_count) ? color_attachments[5]->GetImageView() : nullptr,
                        .imageLayout = (5 < color_target_count) ? color_attachments[5]->GetImageLayout() : nullptr,
                        .resolveMode = VK_RESOLVE_MODE_NONE,
                        .resolveImageView = 0,
                        .resolveImageLayout = 0,
                        .loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                        .storeOp = VK_ATTACHMENT_STORE_OP_STORE
                    },
                    {
                        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                        .imageView = (6 < color_target_count) ? color_attachments[6]->GetImageView() : nullptr,
                        .imageLayout = (6 < color_target_count) ? color_attachments[6]->GetImageLayout() : nullptr,
                        .resolveMode = VK_RESOLVE_MODE_NONE,
                        .resolveImageView = 0,
                        .resolveImageLayout = 0,
                        .loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                        .storeOp = VK_ATTACHMENT_STORE_OP_STORE
                    },
                    {
                        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                        .imageView = (7 < color_target_count) ? color_attachments[7]->GetImageView() : nullptr,
                        .imageLayout = (7 < color_target_count) ? color_attachments[7]->GetImageLayout() : nullptr,
                        .resolveMode = VK_RESOLVE_MODE_NONE,
                        .resolveImageView = 0,
                        .resolveImageLayout = 0,
                        .loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                        .storeOp = VK_ATTACHMENT_STORE_OP_STORE
                    },
                };

                /* Initialize our depth stencil attachment */
                const VkRenderingAttachmentInfo depth_stencil_attachment = {
                    .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                    .imageView = (depth_stencil_attachment != nullptr) ? depth_stencil_attachment->GetImageView() : nullptr,
                    .imageLayout = (depth_stencil_attachment != nullptr) ? depth_stencil_attachment->GetImageLayout() : nullptr,
                    .resolveMode = VK_RESOLVE_MODE_NONE,
                    .resolveImageView = 0,
                    .resolveImageLayout = 0,
                    .loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                    .storeOp = VK_ATTACHMENT_STORE_OP_STORE
                };

                /* Initialize our rendering info */
                const VkRenderingAttachmentInfo *depth_stencil_ref = (depth_stencil_target != nullptr) ? std::addressof(depth_stencil_attachment) : nullptr;
                const VkRenderingInfo rendering_info = {
                    .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
                    .renderArea = { {}, {} },
                    .layerCount = 1,
                    .viewMask = 0,
                    .colorAttachmentCount = color_target_count,
                    .pColorAttachments = color_attachments,
                    .pDepthAttachments = depth_stencil_ref,
                    .pStencilAttechments = depth_stencil_ref
                };

                ::vkCmdBeginRendering(m_vk_command_buffer, std::addressof(rendering_info));
                m_is_rendering = true;
            }
    };
}