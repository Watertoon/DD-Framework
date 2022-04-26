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

    struct ColorBlendCmdState {
        VkLogicOp vk_logic_op;
        float     blend_constants[4];
    };

    struct DepthStencilCmdState {
        bool             depth_test_enable;
        bool             depth_write_enable;
        bool             depth_bounds_test_enable;
        bool             stencil_test_enable;
        VkStencilOpState vk_op_state_front;
        VkStencilOpState vk_op_state_back;
        VkCompareOp      vk_depth_compare_op;
        float            min_depth_bounds;
        float            max_depth_bounds;
    };

    struct RasterizerCmdState {
        VkCullModeFlags vk_cull_mode;
        VkFrontFace     vk_front_face;
        bool            rasterizer_discard_enable;
        bool            depth_bias_enable;
        u16             reserve;
        float           depth_bias_constant_factor;
        float           depth_bias_clamp;
        float           depth_bias_slope_factor;
        float           line_width;
    };

    struct VertexCmdState {
        u32 vertex_binding_count;
        u32 vertex_attribute_count;
        VkVertexInputBindingDescription2EXT *vertex_binding_array;
        VkVertexInputAttributeDescription2EXT *vertex_attribute_array;
    };
    
    struct BarrierCmdState {
        VkPipelineStageFlags2 src_stage_mask;
        VkPipelineStageFlags2 dst_stage_mask;
        VkAccessFlags2        src_access_mask;
        VkAccessFlags2        dst_access_mask;
    };

    class CommandBuffer {
        private:
            friend class Context;
        private:
            VkCommandBuffer         m_vk_command_buffer;
            bool                    m_is_rendering;
            ColorTargetView        *m_color_targets[8];
            DepthStencilTargetView *m_depth_stencil_target;
        private:
            void EndRendering() {
                if (m_is_rendering == true) {
                    m_is_rendering = false;
                    ::vkCmdEndRendering(m_vk_command_buffer);
                }
            }
        public:
            constexpr CommandBuffer(VkCommandBuffer external_handle) : m_is_rendering(false), m_vk_command_buffer(0) { /*...*/ }

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

            /*void ClearColorTarget(ColorTargetView *color_target, const VkClearValue& color) {
                if (m_is_rendering == true) {
                    m_is_rendering = false;
                    ::vkCmdEndRendering(m_vk_command_buffer);
                }

                VkImageSubresourceRange sub_range = {};
                color->BuildSubresourceRange(std::addressof(sub_range));

                ::vkCmdClearColorImage(m_vk_command_buffer, color_target->GetImage(), color_target->GetImageLayout(), std::addressof(color), 1, std::addressof(sub_range));
            }

            void ClearColorTarget(DepthStencilTargetView *depth_stencil_target, const VkClearDepthStencilValue clear_value) {
                if (m_is_rendering == true) {
                    m_is_rendering = false;
                    ::vkCmdEndRendering(m_vk_command_buffer);
                }

                VkImageSubresourceRange sub_range = {};
                depth_stencil_target->BuildSubresourceRange(std::addressof(sub_range));

                ::vkCmdClearDepthStencilImage(m_vk_command_buffer, depth_stencil_target->GetImage(), depth_stencil_target->GetImageLayout(), std::addressof(clear_value), 1, std::addressof(sub_range));
            }*/

            void Draw(VkPrimitiveTopology vk_primitive_topology, u32 vertex_count, u32 base_vertex) {
                ::vkCmdSetPrimitiveTopologyEXT(m_vk_command_buffer, vk_primitive_topology);
                ::vkCmdDraw(m_vk_command_buffer, vertex_count, 1, base_vertex, 0);
            }
            
            void DrawIndexed(VkPrimitiveTopology vk_primitive_topology, VkIndexType index_format, Buffer *index_buffer, u32 index_count, u32 base_index) {
                /* Relocate memory pool to device memory if required */
                if (index_buffer->RequiresRelocation() == true) {
                    index_buffer->Relocate(m_vk_command_buffer);
                }
                
                /* Bind Buffer */
                const VkDeviceSize offset = 0;
                ::vkCmdBindIndexBuffer(m_vk_command_buffer, index_buffer->GetBuffer(), offset, index_format);
                
                /* Draw */
                ::vkCmdSetPrimitiveTopologyEXT(m_vk_command_buffer, vk_primitive_topology);
                ::vkCmdDrawIndexed(m_vk_command_buffer, index_count, 1, base_index, 0, 0);
            }

            void SetPipeline(Pipeline *pipeline) {
                ::vkCmdBindPipeline(m_vk_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->GetPipeline());
            }

            void SetColorBlendState(const ColorBlendCmdState *color_blend_state) {
                ::vkCmdSetLogicOpEXT(m_vk_command_buffer, color_blend_state->vk_logic_op);
                ::vkCmdSetBlendConstants(m_vk_command_buffer, color_blend_state->blend_constants);
            }

            void SetDepthStencilState(const DepthStencilCmdState *depth_stencil_state) {
                ::vkCmdSetDepthTestEnable(m_vk_command_buffer, depth_stencil_state->depth_test_enable);
                ::vkCmdSetDepthWriteEnable(m_vk_command_buffer, depth_stencil_state->depth_write_enable);
                ::vkCmdSetDepthBoundsTestEnable(m_vk_command_buffer, depth_stencil_state->depth_bounds_test_enable);
                ::vkCmdSetDepthBounds(m_vk_command_buffer, depth_stencil_state->min_depth_bounds, depth_stencil_state->max_depth_bounds);
                ::vkCmdSetDepthCompareOp(m_vk_command_buffer, depth_stencil_state->vk_depth_compare_op);
                ::vkCmdSetStencilTestEnable(m_vk_command_buffer, depth_stencil_state->stencil_test_enable);
                ::vkCmdSetStencilCompareMask(m_vk_command_buffer, VK_STENCIL_FACE_FRONT_BIT, depth_stencil_state->vk_op_state_front.compareMask);
                ::vkCmdSetStencilCompareMask(m_vk_command_buffer, VK_STENCIL_FACE_BACK_BIT, depth_stencil_state->vk_op_state_back.compareMask);
                ::vkCmdSetStencilWriteMask(m_vk_command_buffer, VK_STENCIL_FACE_FRONT_BIT, depth_stencil_state->vk_op_state_front.writeMask);
                ::vkCmdSetStencilWriteMask(m_vk_command_buffer, VK_STENCIL_FACE_BACK_BIT, depth_stencil_state->vk_op_state_back.writeMask);
                ::vkCmdSetStencilReference(m_vk_command_buffer, VK_STENCIL_FACE_FRONT_BIT, depth_stencil_state->vk_op_state_front.reference);
                ::vkCmdSetStencilReference(m_vk_command_buffer, VK_STENCIL_FACE_BACK_BIT, depth_stencil_state->vk_op_state_back.reference);
                ::vkCmdSetStencilOp(m_vk_command_buffer, VK_STENCIL_FACE_FRONT_BIT, depth_stencil_state->vk_op_state_front.failOp, depth_stencil_state->vk_op_state_front.passOp, depth_stencil_state->vk_op_state_front.depthFailOp, depth_stencil_state->vk_op_state_front.compareOp);
                ::vkCmdSetStencilOp(m_vk_command_buffer, VK_STENCIL_FACE_BACK_BIT, depth_stencil_state->vk_op_state_back.failOp, depth_stencil_state->vk_op_state_back.passOp, depth_stencil_state->vk_op_state_back.depthFailOp, depth_stencil_state->vk_op_state_back.compareOp);
            }

            void SetRasterizerState(const RasterizerCmdState *rasterizer_state) {
                ::vkCmdSetRasterizerDiscardEnable(m_vk_command_buffer, rasterizer_state->rasterizer_discard_enable);
                ::vkCmdSetDepthBiasEnable(m_vk_command_buffer, rasterizer_state->depth_bias_enable);
                ::vkCmdSetDepthBias(m_vk_command_buffer, rasterizer_state->depth_bias_constant_factor, rasterizer_state->depth_bias_clamp, rasterizer_state->depth_bias_slope_factor);
                ::vkCmdSetLineWidth(m_vk_command_buffer, rasterizer_state->line_width);
                ::vkCmdSetCullMode(m_vk_command_buffer, rasterizer_state->vk_cull_mode);
                ::vkCmdSetFrontFace(m_vk_command_buffer, rasterizer_state->vk_front_face);
            }

            void SetVertexState(const VertexCmdState *vertex_state) {
                ::vkCmdSetVertexInputEXT(m_vk_command_buffer, vertex_state->vertex_binding_count, vertex_state->vertex_binding_array, vertex_state->vertex_attribute_count, vertex_state->vertex_attribute_array);
            }

            void SetViewports(u32 viewport_count, const VkViewport *viewport_array) {
                ::vkCmdSetViewportWithCount(m_vk_command_buffer, viewport_count, viewport_array);
            }

            void SetScissors(u32 scissor_count, const VkRect2D *scissor_array) {
                ::vkCmdSetScissorWithCount(m_vk_command_buffer, scissor_count, scissor_array);
            }

            void SetViewportScissors(u32 viewport_count, const VkViewport *viewport_array, u32 scissor_count, const VkRect2D *scissor_array) {
                ::vkCmdSetViewportWithCount(m_vk_command_buffer, viewport_count, viewport_array);
                ::vkCmdSetScissorWithCount(m_vk_command_buffer, scissor_count, scissor_array);
            }

            void SetRenderTargets(s32 color_target_count, ColorTargetView **color_targets, DepthStencilTargetView *depth_stencil_target) {
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
                        .imageView = (0 < color_target_count) ? color_targets[0]->GetImageView() : nullptr,
                        .imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
                        .resolveMode = VK_RESOLVE_MODE_NONE,
                        .resolveImageView = 0,
                        .resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                        .loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                        .storeOp = VK_ATTACHMENT_STORE_OP_STORE
                    },
                    {
                        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                        .imageView = (1 < color_target_count) ? color_targets[1]->GetImageView() : nullptr,
                        .imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
                        .resolveMode = VK_RESOLVE_MODE_NONE,
                        .resolveImageView = 0,
                        .resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                        .loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                        .storeOp = VK_ATTACHMENT_STORE_OP_STORE
                    },
                    {
                        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                        .imageView = (2 < color_target_count) ? color_targets[2]->GetImageView() : nullptr,
                        .imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
                        .resolveMode = VK_RESOLVE_MODE_NONE,
                        .resolveImageView = 0,
                        .resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                        .loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                        .storeOp = VK_ATTACHMENT_STORE_OP_STORE
                    },
                    {
                        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                        .imageView = (3 < color_target_count) ? color_targets[3]->GetImageView() : nullptr,
                        .imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
                        .resolveMode = VK_RESOLVE_MODE_NONE,
                        .resolveImageView = 0,
                        .resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                        .loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                        .storeOp = VK_ATTACHMENT_STORE_OP_STORE
                    },
                    {
                        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                        .imageView = (4 < color_target_count) ? color_targets[4]->GetImageView() : nullptr,
                        .imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
                        .resolveMode = VK_RESOLVE_MODE_NONE,
                        .resolveImageView = 0,
                        .resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                        .loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                        .storeOp = VK_ATTACHMENT_STORE_OP_STORE
                    },
                    {
                        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                        .imageView = (5 < color_target_count) ? color_targets[5]->GetImageView() : nullptr,
                        .imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
                        .resolveMode = VK_RESOLVE_MODE_NONE,
                        .resolveImageView = 0,
                        .resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                        .loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                        .storeOp = VK_ATTACHMENT_STORE_OP_STORE
                    },
                    {
                        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                        .imageView = (6 < color_target_count) ? color_targets[6]->GetImageView() : nullptr,
                        .imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
                        .resolveMode = VK_RESOLVE_MODE_NONE,
                        .resolveImageView = 0,
                        .resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                        .loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                        .storeOp = VK_ATTACHMENT_STORE_OP_STORE
                    },
                    {
                        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                        .imageView = (7 < color_target_count) ? color_targets[7]->GetImageView() : nullptr,
                        .imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
                        .resolveMode = VK_RESOLVE_MODE_NONE,
                        .resolveImageView = 0,
                        .resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                        .loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                        .storeOp = VK_ATTACHMENT_STORE_OP_STORE
                    }
                };

                /* Initialize our depth stencil attachment */
                const VkRenderingAttachmentInfo depth_stencil_attachment = {
                    .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                    .imageView = (depth_stencil_target != nullptr) ? depth_stencil_target->GetImageView() : nullptr,
                    .imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
                    .resolveMode = VK_RESOLVE_MODE_NONE,
                    .resolveImageView = 0,
                    .resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                    .loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                    .storeOp = VK_ATTACHMENT_STORE_OP_STORE
                };
                
                /* Copy our attachmants for transitions when we need to end rendering later */
                for (u32 i = 0; i < color_target_count; ++i) {
                    m_color_targets[i] = color_targets[i];
                }
                m_depth_stencil_target = depth_stencil_target;

                /* Initialize our rendering info */
                const VkRenderingAttachmentInfo *depth_stencil_ref = (depth_stencil_target != nullptr) ? std::addressof(depth_stencil_attachment) : nullptr;
                const VkRenderingInfo rendering_info = {
                    .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
                    .renderArea = { {}, {} },
                    .layerCount = 1,
                    .viewMask = 0,
                    .colorAttachmentCount = color_target_count,
                    .pColorAttachments = color_attachments,
                    .pDepthAttachment = depth_stencil_ref,
                    .pStencilAttachment = depth_stencil_ref
                };

                ::vkCmdBeginRendering(m_vk_command_buffer, std::addressof(rendering_info));
                m_is_rendering = true;
            }
            
            void SetVertexBuffer(u32 binding, const Buffer *buffer, VkDeviceSize stride, VkDeviceSize size) {
                const VkBuffer vk_buffer = buffer->GetBuffer();
                const VkDeviceSize offset = 0;
                ::vkCmdBindVertexBuffers2(m_vk_command_buffer, binding, 1, std::addressof(vk_buffer), std::addressof(offset), std::addressof(size), std::addressof(stride));
            }
            
            void SetBufferStateTransition(Buffer *buffer, const BarrierCmdState *barrier_state) {
                /* Buffer memory barrier */
                const VkBufferMemoryBarrier2 buffer_barrier = {
                    .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
                    .srcStageMask = barrier_state->src_stage_mask,
                    .srcAccessMask = barrier_state->dst_stage_mask,
                    .dstStageMask = barrier_state->src_access_mask,
                    .dstAccessMask = barrier_state->dst_access_mask,
                    .buffer = buffer->GetBuffer(),
                    .size = VK_WHOLE_SIZE
                };

                const VkDependencyInfo dependency_info = {
                    .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
                    .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT,
                    .bufferMemoryBarrierCount = 1,
                    .pBufferMemoryBarriers = std::addressof(buffer_barrier)
                };

                ::vkCmdPipelineBarrier2(m_vk_command_buffer, std::addressof(dependency_info));
            }
            
            void SetTextureStateTransition(Texture *texture, const BarrierCmdState *barrier_state, VkImageAspectFlagBits aspect_mask) {
                /* Image memory barrier */
                const VkImageMemoryBarrier2 buffer_barrier = {
                    .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                    .srcStageMask = barrier_state->src_stage_mask,
                    .srcAccessMask = barrier_state->dst_stage_mask,
                    .dstStageMask = barrier_state->src_access_mask,
                    .dstAccessMask = barrier_state->dst_access_mask,
                    .image = texture->GetImage(),
                    .subresourceRange = {
                        .aspectMask = aspect_mask,
                        .baseMipLevel = 0,
                        .levelCount = VK_REMAINING_MIP_LEVELS,
                        .baseArrayLayer = 0,
                        .layerCount = VK_REMAINING_ARRAY_LAYERS
                    }
                };

                const VkDependencyInfo dependency_info = {
                    .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
                    .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT,
                    .imageMemoryBarrierCount = 1,
                    .pImageMemoryBarriers = std::addressof(buffer_barrier)
                };

                ::vkCmdPipelineBarrier2(m_vk_command_buffer, std::addressof(dependency_info));
            }
    };
}