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
#include <dd.hpp>

namespace dd::vk {

    /* Private */

    void CommandBuffer::BeginRenderingIfNotRendering() {

        if (m_are_new_targets == true) {
            m_are_new_targets = false;
            this->EndRenderingIfRendering();
        } else if (m_is_rendering == true) {
            return;
        }

        /* Initialize color target attachments */
        VkRenderingAttachmentInfo color_attachments[8] = {};
        u32 color_target_count = 0;
        for (const ColorTargetView *color_target : m_color_targets) {

            if (color_target == nullptr) {
                break;
            }

            color_attachments[color_target_count].sType              = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
            color_attachments[color_target_count].imageView          = color_target->GetImageView();
            color_attachments[color_target_count].imageLayout        = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR;
            color_attachments[color_target_count].resolveMode        = VK_RESOLVE_MODE_NONE;
            color_attachments[color_target_count].resolveImageView   = 0;
            color_attachments[color_target_count].resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            color_attachments[color_target_count].loadOp             = (m_fast_clear_color == true) ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            color_attachments[color_target_count].storeOp            = VK_ATTACHMENT_STORE_OP_STORE;
            color_attachments[color_target_count].clearValue         = { m_vk_clear_color.float32[0], m_vk_clear_color.float32[1],  m_vk_clear_color.float32[2], m_vk_clear_color.float32[3] };
            color_target_count += 1;
        }

        /* Initialize depth stencil attachment */
        VkRenderingAttachmentInfo depth_stencil_attachment = {
            .sType              = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
            .imageView          = (m_depth_stencil_target != nullptr) ? m_depth_stencil_target->GetImageView() : nullptr,
            .imageLayout        = VK_IMAGE_LAYOUT_GENERAL,
            .resolveMode        = VK_RESOLVE_MODE_NONE,
            .resolveImageView   = 0,
            .resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .loadOp             = (m_fast_clear_depth_stencil == true) ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .storeOp            = VK_ATTACHMENT_STORE_OP_STORE
        };
        depth_stencil_attachment.clearValue.depthStencil = m_vk_clear_depth_stencil;

        /* Initialize rendering info */
        const VkRenderingAttachmentInfo *depth_stencil_ref = (m_depth_stencil_target != nullptr) ? std::addressof(depth_stencil_attachment) : nullptr;
        u32 width = 0; 
        u32 height = 0;
        vk::GetGlobalContext()->GetWindowDimensionsUnsafe(std::addressof(width), std::addressof(height));
        const VkRenderingInfo rendering_info = {
            .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
            .renderArea = { 
                {
                    .x = 0,
                    .y = 0
                }, 
                {
                    .width = width,
                    .height = height
                } 
            },
            .layerCount           = 1,
            .viewMask             = 0,
            .colorAttachmentCount = color_target_count,
            .pColorAttachments    = color_attachments,
            .pDepthAttachment     = depth_stencil_ref,
            .pStencilAttachment   = depth_stencil_ref
        };

        ::pfn_vkCmdBeginRendering(m_vk_command_buffer, std::addressof(rendering_info));
        m_is_rendering = true;
    }

    void CommandBuffer::EndRenderingIfRendering() {
        if (m_is_rendering == true) {
            m_is_rendering = false;
            ::pfn_vkCmdEndRendering(m_vk_command_buffer);
        }
    }

    void CommandBuffer::UpdateResourceBufferIfNecessary() {
        DD_ASSERT(m_resource_buffer_mapped_address != nullptr);
        
        /* Map and copy resource data for necessary stages */
        if (m_need_vertex_resource_update == true) {
            ::memcpy(reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(m_resource_buffer_mapped_address) + (sizeof(ResourceBuffer) * m_resource_update_count)), std::addressof(m_resource_buffer_per_stage_array[ShaderStage_Vertex]), sizeof(ResourceBuffer));
            const VkDeviceAddress offset_address = m_vk_resource_buffer_address + (sizeof(ResourceBuffer) * m_resource_update_count);
            ::pfn_vkCmdPushConstants(m_vk_command_buffer, GetGlobalContext()->GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, sizeof(VkDeviceAddress) * 0, sizeof(VkDeviceAddress), std::addressof(offset_address));
            m_resource_update_count += 1;
            m_need_vertex_resource_update = false;
        }
        if (m_need_tessellation_control_resource_update == true) {
            ::memcpy(reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(m_resource_buffer_mapped_address) + (sizeof(ResourceBuffer) * m_resource_update_count)), std::addressof(m_resource_buffer_per_stage_array[ShaderStage_TessellationControl]), sizeof(ResourceBuffer));
            const VkDeviceAddress offset_address = m_vk_resource_buffer_address + (sizeof(ResourceBuffer) * m_resource_update_count);
            ::pfn_vkCmdPushConstants(m_vk_command_buffer, GetGlobalContext()->GetPipelineLayout(), VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, sizeof(VkDeviceAddress) * 1, sizeof(VkDeviceAddress), std::addressof(offset_address));
            m_resource_update_count += 1;
            m_need_tessellation_control_resource_update = false;
        }
        if (m_need_tessellation_evaluation_resource_update == true) {
            ::memcpy(reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(m_resource_buffer_mapped_address) + (sizeof(ResourceBuffer) * m_resource_update_count)), std::addressof(m_resource_buffer_per_stage_array[ShaderStage_TessellationEvaluation]), sizeof(ResourceBuffer));
            const VkDeviceAddress offset_address = m_vk_resource_buffer_address + (sizeof(ResourceBuffer) * m_resource_update_count);
            ::pfn_vkCmdPushConstants(m_vk_command_buffer, GetGlobalContext()->GetPipelineLayout(), VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, sizeof(VkDeviceAddress) * 2, sizeof(VkDeviceAddress), std::addressof(offset_address));
            m_resource_update_count += 1;
            m_need_tessellation_evaluation_resource_update = false;
        }
        if (m_need_geometry_resource_update == true) {
            ::memcpy(reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(m_resource_buffer_mapped_address) + (sizeof(ResourceBuffer) * m_resource_update_count)), std::addressof(m_resource_buffer_per_stage_array[ShaderStage_Geometry]), sizeof(ResourceBuffer));
            const VkDeviceAddress offset_address = m_vk_resource_buffer_address + (sizeof(ResourceBuffer) * m_resource_update_count);
            ::pfn_vkCmdPushConstants(m_vk_command_buffer, GetGlobalContext()->GetPipelineLayout(), VK_SHADER_STAGE_GEOMETRY_BIT, sizeof(VkDeviceAddress) * 3, sizeof(VkDeviceAddress), std::addressof(offset_address));
            m_resource_update_count += 1;
            m_need_geometry_resource_update = false;
        }
        if (m_need_fragment_resource_update == true) {
            ::memcpy(reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(m_resource_buffer_mapped_address) + (sizeof(ResourceBuffer) * m_resource_update_count)), std::addressof(m_resource_buffer_per_stage_array[ShaderStage_Fragment]), sizeof(ResourceBuffer));
            const VkDeviceAddress offset_address = m_vk_resource_buffer_address + (sizeof(ResourceBuffer) * m_resource_update_count);
            ::pfn_vkCmdPushConstants(m_vk_command_buffer, GetGlobalContext()->GetPipelineLayout(), VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(VkDeviceAddress) * 4, sizeof(VkDeviceAddress), std::addressof(offset_address));
            m_resource_update_count += 1;
            m_need_fragment_resource_update = false;
        }
        if (m_need_compute_resource_update == true) {
            ::memcpy(reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(m_resource_buffer_mapped_address) + (sizeof(ResourceBuffer) * m_resource_update_count)), std::addressof(m_resource_buffer_per_stage_array[ShaderStage_Compute]), sizeof(ResourceBuffer));
            const VkDeviceAddress offset_address = m_vk_resource_buffer_address + (sizeof(ResourceBuffer) * m_resource_update_count);
            ::pfn_vkCmdPushConstants(m_vk_command_buffer, GetGlobalContext()->GetPipelineLayout(), VK_SHADER_STAGE_COMPUTE_BIT, sizeof(VkDeviceAddress) * 5, sizeof(VkDeviceAddress), std::addressof(offset_address));
            m_resource_update_count += 1;
            m_need_compute_resource_update = false;
        }
    }

    /* Public */

    void CommandBuffer::Initialize(const Context *context, size_t draw_memory_size) {

        /* Allocate Commmand Buffer */
        const VkCommandBufferAllocateInfo allocate_info = {
            .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool        = context->GetGraphicsCommandPool(),
            .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1
        };

        const u32 result0 = ::pfn_vkAllocateCommandBuffers(context->GetDevice(), std::addressof(allocate_info), std::addressof(m_vk_command_buffer));
        DD_ASSERT(result0 == VK_SUCCESS);

        /* Allocate resource buffer memory */
        const s32 device_memory_type = context->FindMemoryHeapIndex(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        DD_ASSERT(device_memory_type != -1);

        const VkMemoryAllocateFlagsInfo device_allocate_flags = {
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO,
            .flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT
        };
        const VkMemoryAllocateInfo device_allocate_info =  {
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .pNext = std::addressof(device_allocate_flags),
            .allocationSize = draw_memory_size,
            .memoryTypeIndex = static_cast<u32>(device_memory_type)
        };
        const u32 result3 = ::pfn_vkAllocateMemory(context->GetDevice(), std::addressof(device_allocate_info), nullptr, std::addressof(m_vk_resource_buffer_memory));
        DD_ASSERT(result3 == VK_SUCCESS);

        /* Create resource buffers for each stage */
        const u32 queue_family_index = context->GetGraphicsQueueFamilyIndex();
        const VkBufferCreateInfo buffer_create_info = {
            .sType                 = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size                  = draw_memory_size,
            .usage                 = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
            .sharingMode           = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 1,
            .pQueueFamilyIndices   = std::addressof(queue_family_index)
        };

        const u32 result4 = ::pfn_vkCreateBuffer(context->GetDevice(), std::addressof(buffer_create_info), nullptr, std::addressof(m_vk_resource_buffer));
        DD_ASSERT(result4 == VK_SUCCESS);

        /* Bind resource buffer memory to resource buffers */
        const u32 result10 = ::pfn_vkBindBufferMemory(context->GetDevice(), m_vk_resource_buffer, m_vk_resource_buffer_memory, 0);
        DD_ASSERT(result10 == VK_SUCCESS);

        /* Map the resource buffer memory */
        const u32 result16 = ::pfn_vkMapMemory(context->GetDevice(), m_vk_resource_buffer_memory, 0, VK_WHOLE_SIZE , 0, std::addressof(m_resource_buffer_mapped_address));
        DD_ASSERT(result16 == VK_SUCCESS);

        /* Store the device address */
        const VkBufferDeviceAddressInfo address_info = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
            .buffer = m_vk_resource_buffer
        };
        m_vk_resource_buffer_address = ::pfn_vkGetBufferDeviceAddress(context->GetDevice(), std::addressof(address_info));
    }

    void CommandBuffer::Finalize(const Context *context) {

        ::pfn_vkFreeCommandBuffers(context->GetDevice(), context->GetGraphicsCommandPool(), 1, std::addressof(m_vk_command_buffer));

        ::pfn_vkDestroyBuffer(context->GetDevice(), m_vk_resource_buffer, nullptr);

        if (m_resource_buffer_mapped_address != nullptr) {
            ::pfn_vkUnmapMemory(GetGlobalContext()->GetDevice(), m_vk_resource_buffer_memory);
            m_resource_buffer_mapped_address = nullptr;
        }

        ::pfn_vkFreeMemory(context->GetDevice(), m_vk_resource_buffer_memory, nullptr);
    }

    void CommandBuffer::Begin() {

        /* Begin Command buffer */
        const VkCommandBufferBeginInfo begin_info = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
        };

        const u32 result = ::pfn_vkBeginCommandBuffer(m_vk_command_buffer, std::addressof(begin_info));
        DD_ASSERT(result == VK_SUCCESS);

        m_resource_buffer_per_stage_array[ShaderStage_Vertex].SetDefaults();
        m_resource_buffer_per_stage_array[ShaderStage_TessellationControl].SetDefaults();
        m_resource_buffer_per_stage_array[ShaderStage_TessellationEvaluation].SetDefaults();
        m_resource_buffer_per_stage_array[ShaderStage_Geometry].SetDefaults();
        m_resource_buffer_per_stage_array[ShaderStage_Fragment].SetDefaults();
        m_resource_buffer_per_stage_array[ShaderStage_Compute].SetDefaults();
        m_resource_update_count = 0;
    }

    void CommandBuffer::End() {
        this->EndRenderingIfRendering();

        const u32 result = ::pfn_vkEndCommandBuffer(m_vk_command_buffer);
        DD_ASSERT(result == VK_SUCCESS);
    }

    void CommandBuffer::ClearColorTarget(ColorTargetView *color_target, const VkClearColorValue* color, const VkImageSubresourceRange *sub_range) {

        this->EndRenderingIfRendering();

        ::pfn_vkCmdClearColorImage(m_vk_command_buffer, color_target->GetImage(), VK_IMAGE_LAYOUT_GENERAL, color, 1, sub_range);

        /* Barrier the clear */
        const dd::vk::TextureBarrierCmdState clear_barrier_state = {
            .vk_src_stage_mask  = VK_PIPELINE_STAGE_TRANSFER_BIT,
            .vk_dst_stage_mask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .vk_src_access_mask = VK_ACCESS_TRANSFER_WRITE_BIT,
            .vk_dst_access_mask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            .vk_src_layout      = VK_IMAGE_LAYOUT_GENERAL,
            .vk_dst_layout      = VK_IMAGE_LAYOUT_GENERAL
        };
        this->SetTextureStateTransition(color_target->GetTexture(), std::addressof(clear_barrier_state), VK_IMAGE_ASPECT_COLOR_BIT);
    }

    void CommandBuffer::ClearDepthStencilTarget(DepthStencilTargetView *depth_stencil_target, const VkClearDepthStencilValue clear_value, const VkImageSubresourceRange *sub_range) {

        this->EndRenderingIfRendering();

        ::pfn_vkCmdClearDepthStencilImage(m_vk_command_buffer, depth_stencil_target->GetImage(), VK_IMAGE_LAYOUT_GENERAL, std::addressof(clear_value), 1, sub_range);

        /* Barrier the clear */
        const dd::vk::TextureBarrierCmdState clear_barrier_state = {
            .vk_src_stage_mask  = VK_PIPELINE_STAGE_TRANSFER_BIT,
            .vk_dst_stage_mask  = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
            .vk_src_access_mask = VK_ACCESS_TRANSFER_WRITE_BIT,
            .vk_dst_access_mask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            .vk_src_layout      = VK_IMAGE_LAYOUT_GENERAL,
            .vk_dst_layout      = VK_IMAGE_LAYOUT_GENERAL
        };
        this->SetTextureStateTransition(depth_stencil_target->GetTexture(), std::addressof(clear_barrier_state), static_cast<VkImageAspectFlagBits>(VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT));
    }

    void CommandBuffer::RegisterFastClearColor(const VkClearColorValue& color_value) {
        m_vk_clear_color.float32[0] = color_value.float32[0];
        m_vk_clear_color.float32[1] = color_value.float32[1];
        m_vk_clear_color.float32[2] = color_value.float32[2];
        m_vk_clear_color.float32[3] = color_value.float32[3];
        m_fast_clear_color          = true;
    }

    void CommandBuffer::RegisterFastClearDepthStencil(const VkClearDepthStencilValue& depth_stencil_value) {
        m_vk_clear_depth_stencil.depth   = depth_stencil_value.depth;
        m_vk_clear_depth_stencil.stencil = depth_stencil_value.stencil;
        m_fast_clear_depth_stencil  = true;
    }

    void CommandBuffer::Draw(VkPrimitiveTopology vk_primitive_topology, u32 vertex_count, u32 base_vertex) {

        this->UpdateResourceBufferIfNecessary();
        this->BeginRenderingIfNotRendering();

        /* Draw */
        ::pfn_vkCmdSetPrimitiveTopology(m_vk_command_buffer, vk_primitive_topology);
        ::pfn_vkCmdDraw(m_vk_command_buffer, vertex_count, 1, base_vertex, 0);
    }

    void CommandBuffer::DrawIndexed(VkPrimitiveTopology vk_primitive_topology, VkIndexType index_format, Buffer *index_buffer, u32 index_count, u32 base_index) {

        /* Relocate memory pool to device memory if required */
        if (index_buffer->RequiresRelocation() == true) {
            this->EndRenderingIfRendering();
            index_buffer->Relocate(m_vk_command_buffer);
        }
    
        this->UpdateResourceBufferIfNecessary();
        this->BeginRenderingIfNotRendering();

        /* Bind index buffer */
        const VkDeviceSize offset = 0;
        ::pfn_vkCmdBindIndexBuffer(m_vk_command_buffer, index_buffer->GetBuffer(), offset, index_format);

        /* Draw */
        ::pfn_vkCmdSetPrimitiveTopology(m_vk_command_buffer, vk_primitive_topology);
        ::pfn_vkCmdDrawIndexed(m_vk_command_buffer, index_count, 1, base_index, 0, 0);
    }

    void CommandBuffer::DrawInstanced(VkPrimitiveTopology vk_primitive_topology, u32 vertex_count, u32 base_vertex, u32 instance_count, u32 base_instance) {

        this->UpdateResourceBufferIfNecessary();
        this->BeginRenderingIfNotRendering();

        /* Draw */
        ::pfn_vkCmdSetPrimitiveTopology(m_vk_command_buffer, vk_primitive_topology);
        ::pfn_vkCmdDraw(m_vk_command_buffer, vertex_count, instance_count, base_vertex, base_instance);
    }

    void CommandBuffer::DrawInstancedIndexed(VkPrimitiveTopology vk_primitive_topology, VkIndexType index_format, Buffer *index_buffer, u32 index_count, u32 base_index, u32 instance_count, u32 base_instance) {

        /* Relocate memory pool to device memory if required */
        if (index_buffer->RequiresRelocation() == true) {
            this->EndRenderingIfRendering();
            index_buffer->Relocate(m_vk_command_buffer);
        }
    
        this->UpdateResourceBufferIfNecessary();
        this->BeginRenderingIfNotRendering();

        /* Bind index buffer */
        const VkDeviceSize offset = 0;
        ::pfn_vkCmdBindIndexBuffer(m_vk_command_buffer, index_buffer->GetBuffer(), offset, index_format);

        /* Draw */
        ::pfn_vkCmdSetPrimitiveTopology(m_vk_command_buffer, vk_primitive_topology);
        ::pfn_vkCmdDrawIndexed(m_vk_command_buffer, index_count, instance_count, base_index, 0, base_instance);
    }

    void CommandBuffer::SetShader(Shader *shader) {
        ::pfn_vkCmdBindPipeline(m_vk_command_buffer, shader->GetPipelineBindPoint(), shader->GetPipeline());
    }

    void CommandBuffer::SetPipelineState(const PipelineCmdState *pipeline_state) {
        this->SetColorBlendState(std::addressof(pipeline_state->color_blend_state));
        this->SetDepthStencilState(std::addressof(pipeline_state->depth_stencil_state));
        this->SetRasterizerState(std::addressof(pipeline_state->rasterizer_state));
        this->SetVertexState(std::addressof(pipeline_state->vertex_state));
    }

    void CommandBuffer::SetColorBlendState(const ColorBlendCmdState *color_blend_state) {
        ::pfn_vkCmdSetLogicOpEnableEXT(m_vk_command_buffer, color_blend_state->logic_op_enable);
        ::pfn_vkCmdSetLogicOpEXT(m_vk_command_buffer, color_blend_state->vk_logic_op);
        ::pfn_vkCmdSetBlendConstants(m_vk_command_buffer, color_blend_state->blend_constants);
        
        /* Color Blend */
        ::pfn_vkCmdSetColorBlendEnableEXT(m_vk_command_buffer, 0, color_blend_state->color_blend_count, color_blend_state->color_blend_enables);
        ::pfn_vkCmdSetColorBlendEquationEXT(m_vk_command_buffer, 0, color_blend_state->color_blend_count, color_blend_state->vk_color_blend_equations);
        ::pfn_vkCmdSetColorWriteMaskEXT(m_vk_command_buffer, 0, color_blend_state->color_blend_count, color_blend_state->vk_color_blend_write_masks);
    }

    void CommandBuffer::SetDepthStencilState(const DepthStencilCmdState *depth_stencil_state) {
        /* Depth */
        ::pfn_vkCmdSetDepthTestEnable(m_vk_command_buffer, depth_stencil_state->depth_test_enable);
        ::pfn_vkCmdSetDepthWriteEnable(m_vk_command_buffer, depth_stencil_state->depth_write_enable);
        ::pfn_vkCmdSetDepthBoundsTestEnable(m_vk_command_buffer, depth_stencil_state->depth_bounds_test_enable);
        ::pfn_vkCmdSetDepthBounds(m_vk_command_buffer, depth_stencil_state->min_depth_bounds, depth_stencil_state->max_depth_bounds);
        ::pfn_vkCmdSetDepthCompareOp(m_vk_command_buffer, depth_stencil_state->vk_depth_compare_op);

        /* Stencil */
        ::pfn_vkCmdSetStencilTestEnable(m_vk_command_buffer, depth_stencil_state->stencil_test_enable);
        ::pfn_vkCmdSetStencilCompareMask(m_vk_command_buffer, VK_STENCIL_FACE_FRONT_BIT, depth_stencil_state->vk_op_state_front.compareMask);
        ::pfn_vkCmdSetStencilCompareMask(m_vk_command_buffer, VK_STENCIL_FACE_BACK_BIT, depth_stencil_state->vk_op_state_back.compareMask);
        ::pfn_vkCmdSetStencilWriteMask(m_vk_command_buffer, VK_STENCIL_FACE_FRONT_BIT, depth_stencil_state->vk_op_state_front.writeMask);
        ::pfn_vkCmdSetStencilWriteMask(m_vk_command_buffer, VK_STENCIL_FACE_BACK_BIT, depth_stencil_state->vk_op_state_back.writeMask);
        ::pfn_vkCmdSetStencilReference(m_vk_command_buffer, VK_STENCIL_FACE_FRONT_BIT, depth_stencil_state->vk_op_state_front.reference);
        ::pfn_vkCmdSetStencilReference(m_vk_command_buffer, VK_STENCIL_FACE_BACK_BIT, depth_stencil_state->vk_op_state_back.reference);
        ::pfn_vkCmdSetStencilOp(m_vk_command_buffer, VK_STENCIL_FACE_FRONT_BIT, depth_stencil_state->vk_op_state_front.failOp, depth_stencil_state->vk_op_state_front.passOp, depth_stencil_state->vk_op_state_front.depthFailOp, depth_stencil_state->vk_op_state_front.compareOp);
        ::pfn_vkCmdSetStencilOp(m_vk_command_buffer, VK_STENCIL_FACE_BACK_BIT, depth_stencil_state->vk_op_state_back.failOp, depth_stencil_state->vk_op_state_back.passOp, depth_stencil_state->vk_op_state_back.depthFailOp, depth_stencil_state->vk_op_state_back.compareOp);
    }

    void CommandBuffer::SetRasterizerState(const RasterizerCmdState *rasterizer_state) {
        ::pfn_vkCmdSetRasterizerDiscardEnable(m_vk_command_buffer, rasterizer_state->rasterizer_discard_enable);
        ::pfn_vkCmdSetPrimitiveRestartEnable(m_vk_command_buffer, rasterizer_state->primitive_restart_enable);
        ::pfn_vkCmdSetDepthBiasEnable(m_vk_command_buffer, rasterizer_state->depth_bias_enable);
        ::pfn_vkCmdSetDepthBias(m_vk_command_buffer, rasterizer_state->depth_bias_constant_factor, rasterizer_state->depth_bias_clamp, rasterizer_state->depth_bias_slope_factor);
        ::pfn_vkCmdSetLineWidth(m_vk_command_buffer, rasterizer_state->line_width);
        ::pfn_vkCmdSetCullMode(m_vk_command_buffer, rasterizer_state->vk_cull_mode);
        ::pfn_vkCmdSetFrontFace(m_vk_command_buffer, rasterizer_state->vk_front_face);
        
        ::pfn_vkCmdSetDepthClampEnableEXT(m_vk_command_buffer, rasterizer_state->depth_clamp_enable);
        ::pfn_vkCmdSetPolygonModeEXT(m_vk_command_buffer, rasterizer_state->vk_polygon_mode);
    }

    void CommandBuffer::SetVertexState(const VertexCmdState *vertex_state) {
        pfn_vkCmdSetVertexInputEXT(m_vk_command_buffer, vertex_state->vertex_binding_count, vertex_state->vertex_binding_array, vertex_state->vertex_attribute_count, vertex_state->vertex_attribute_array);
    }

    void CommandBuffer::SetViewports(u32 viewport_count, const VkViewport *viewport_array) {
        ::pfn_vkCmdSetViewportWithCount(m_vk_command_buffer, viewport_count, viewport_array);
    }

    void CommandBuffer::SetScissors(u32 scissor_count, const VkRect2D *scissor_array) {
        ::pfn_vkCmdSetScissorWithCount(m_vk_command_buffer, scissor_count, scissor_array);
    }

    void CommandBuffer::SetViewportScissors(u32 viewport_count, const VkViewport *viewport_array, u32 scissor_count, const VkRect2D *scissor_array) {
        ::pfn_vkCmdSetViewportWithCount(m_vk_command_buffer, viewport_count, viewport_array);
        ::pfn_vkCmdSetScissorWithCount(m_vk_command_buffer, scissor_count, scissor_array);
    }

    void CommandBuffer::SetRenderTargets(s32 color_target_count, ColorTargetView **color_targets, DepthStencilTargetView *depth_stencil_target) {

        /* Copy the attachmants for when we need to do rendering later */
        for (s32 i = 0; i < color_target_count; ++i) {
            m_color_targets[i] = color_targets[i];
        }
        m_depth_stencil_target = depth_stencil_target;

        /* Only 8 color attachments are supported at time */
        DD_ASSERT(0 < color_target_count && color_target_count <= 8);

        m_are_new_targets = true;
    }

    void CommandBuffer::SetVertexBuffer(u32 binding, Buffer *vertex_buffer, size_t stride, size_t size) {

        /* Relocate memory pool to device memory if required */
        if (vertex_buffer->RequiresRelocation() == true) {
            this->EndRenderingIfRendering();
            vertex_buffer->Relocate(m_vk_command_buffer);
        }

        const VkBuffer vk_buffer = vertex_buffer->GetBuffer();
        const VkDeviceSize offset = 0;
        ::pfn_vkCmdBindVertexBuffers2(m_vk_command_buffer, binding, 1, std::addressof(vk_buffer), std::addressof(offset), std::addressof(size), std::addressof(stride));
    }

    void CommandBuffer::SetBufferStateTransition(Buffer *buffer, const BufferBarrierCmdState *barrier_state) {

        /* Relocate memory pool to device memory if required */
        if (buffer->RequiresRelocation() == true) {
            this->EndRenderingIfRendering();
            buffer->Relocate(m_vk_command_buffer);
        }

        /* Buffer memory barrier */
        const VkBufferMemoryBarrier2 buffer_barrier = {
            .sType         = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
            .srcStageMask  = barrier_state->vk_src_stage_mask,
            .srcAccessMask = barrier_state->vk_src_access_mask,
            .dstStageMask  = barrier_state->vk_dst_stage_mask,
            .dstAccessMask = barrier_state->vk_dst_access_mask,
            .buffer        = buffer->GetBuffer(),
            .size          = VK_WHOLE_SIZE
        };

        const VkDependencyInfo dependency_info = {
            .sType                    = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
            .dependencyFlags          = VK_DEPENDENCY_BY_REGION_BIT,
            .bufferMemoryBarrierCount = 1,
            .pBufferMemoryBarriers    = std::addressof(buffer_barrier)
        };

        ::pfn_vkCmdPipelineBarrier2(m_vk_command_buffer, std::addressof(dependency_info));
    }

    void CommandBuffer::SetTextureStateTransition(Texture *texture, const TextureBarrierCmdState *barrier_state, VkImageAspectFlagBits aspect_mask) {

        if (barrier_state->vk_dst_stage_mask == VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT) {
            this->EndRenderingIfRendering();
        }

        /* Image memory barrier */
        const VkImageMemoryBarrier2 buffer_barrier = {
            .sType         = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .srcStageMask  = barrier_state->vk_src_stage_mask,
            .srcAccessMask = barrier_state->vk_src_access_mask,
            .dstStageMask  = barrier_state->vk_dst_stage_mask,
            .dstAccessMask = barrier_state->vk_dst_access_mask,
            .oldLayout     = barrier_state->vk_src_layout,
            .newLayout     = barrier_state->vk_dst_layout,
            .image         = texture->GetImage(),
            .subresourceRange = {
                .aspectMask     = aspect_mask,
                .baseMipLevel   = 0,
                .levelCount     = VK_REMAINING_MIP_LEVELS,
                .baseArrayLayer = 0,
                .layerCount     = VK_REMAINING_ARRAY_LAYERS
            }
        };

        const VkDependencyInfo dependency_info = {
            .sType                   = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
            .dependencyFlags         = VK_DEPENDENCY_BY_REGION_BIT,
            .imageMemoryBarrierCount = 1,
            .pImageMemoryBarriers    = std::addressof(buffer_barrier)
        };

        ::pfn_vkCmdPipelineBarrier2(m_vk_command_buffer, std::addressof(dependency_info));
        
        texture->SetImageLayout(barrier_state->vk_dst_layout);
    }
    
    void CommandBuffer::SetDescriptorPool(const DescriptorPool *descriptor_pool) {

        VkDescriptorSet vk_descriptor_set = descriptor_pool->GetDescriptorSet();
        u32 set = 0;
        const u32 pool_type = descriptor_pool->GetPoolType();

        if (pool_type == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE) {
            set = 0;
        } else if (pool_type == VK_DESCRIPTOR_TYPE_SAMPLER) {
            set = 1;
        } else {
            DD_ASSERT(false);
        }

        ::pfn_vkCmdBindDescriptorSets(m_vk_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, GetGlobalContext()->GetPipelineLayout(), set, 1, std::addressof(vk_descriptor_set), 0, nullptr);
    }

    void CommandBuffer::SetUniformBuffer(u32 location, ShaderStage shader_stage, const VkDeviceAddress gpu_address) {

        if (m_resource_buffer_per_stage_array[shader_stage].uniform_buffers[location] == gpu_address) {
            return;
        }

        m_resource_buffer_per_stage_array[shader_stage].uniform_buffers[location] = gpu_address;
        std::addressof(m_need_vertex_resource_update)[shader_stage] = true;
    }    

    void CommandBuffer::SetStorageBuffer(u32 location, ShaderStage shader_stage, const VkDeviceAddress gpu_address) {

        if (m_resource_buffer_per_stage_array[shader_stage].storage_buffers[location] == gpu_address) {
            return;
        }

        m_resource_buffer_per_stage_array[shader_stage].storage_buffers[location] = gpu_address;
        std::addressof(m_need_vertex_resource_update)[shader_stage] = true;
    }

    void CommandBuffer::SetTextureAndSampler(u32 location, ShaderStage shader_stage, const DescriptorSlot texture_slot, const DescriptorSlot sampler_slot) {

        if (m_resource_buffer_per_stage_array[shader_stage].texture_ids[location] == texture_slot && m_resource_buffer_per_stage_array[shader_stage].sampler_ids[location] == sampler_slot) {
            return;
        }

        m_resource_buffer_per_stage_array[shader_stage].texture_ids[location] = texture_slot;
        m_resource_buffer_per_stage_array[shader_stage].sampler_ids[location] = sampler_slot;
        std::addressof(m_need_vertex_resource_update)[shader_stage] = true;
    }
}
