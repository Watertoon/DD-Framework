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
            color_attachments[color_target_count].loadOp             = (m_fast_clear == true) ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            color_attachments[color_target_count].storeOp            = VK_ATTACHMENT_STORE_OP_STORE;
            color_attachments[color_target_count].clearValue         = { m_vk_clear_color.float32[0], m_vk_clear_color.float32[1],  m_vk_clear_color.float32[2], m_vk_clear_color.float32[3] };
            color_target_count += 1;
        }

        /* Initialize depth stencil attachment */
        const VkRenderingAttachmentInfo depth_stencil_attachment = {
            .sType              = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
            .imageView          = (m_depth_stencil_target != nullptr) ? m_depth_stencil_target->GetImageView() : nullptr,
            .imageLayout        = VK_IMAGE_LAYOUT_GENERAL,
            .resolveMode        = VK_RESOLVE_MODE_NONE,
            .resolveImageView   = 0,
            .resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .loadOp             = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .storeOp            = VK_ATTACHMENT_STORE_OP_STORE
        };

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
        
        /* Map and copy resource data for necessary stages */
        if (m_need_vertex_resource_update == true) {

            void *memory_address = nullptr;
            const u32 result0 = ::pfn_vkMapMemory(GetGlobalContext()->GetDevice(), m_vk_resource_buffer_memory, VertexBufferMemoryOffset + (sizeof(ResourceBuffer) * m_vertex_resource_update_count), sizeof(ResourceBuffer), 0, std::addressof(memory_address));
            DD_ASSERT(result0 == VK_SUCCESS);

            ::memcpy(memory_address, std::addressof(m_resource_buffer_per_stage_array[ShaderStage_Vertex]), sizeof(ResourceBuffer));

            ::pfn_vkUnmapMemory(GetGlobalContext()->GetDevice(), m_vk_resource_buffer_memory);

            m_vertex_resource_update_count += 1;
        }
        if (m_need_tessellation_evaluation_resource_update == true) {

            void *memory_address = nullptr;
            const u32 result0 = ::pfn_vkMapMemory(GetGlobalContext()->GetDevice(), m_vk_resource_buffer_memory, TessellationEvaluationBufferMemoryOffset + (sizeof(ResourceBuffer) * m_tessellation_evaluation_resource_update_count), sizeof(ResourceBuffer), 0, std::addressof(memory_address));
            DD_ASSERT(result0 == VK_SUCCESS);

            ::memcpy(memory_address, std::addressof(m_resource_buffer_per_stage_array[ShaderStage_TessellationEvaluation]), sizeof(ResourceBuffer));

            ::pfn_vkUnmapMemory(GetGlobalContext()->GetDevice(), m_vk_resource_buffer_memory);

            m_tessellation_evaluation_resource_update_count += 1;
        }
        if (m_need_tessellation_control_resource_update == true) {

            void *memory_address = nullptr;
            const u32 result0 = ::pfn_vkMapMemory(GetGlobalContext()->GetDevice(), m_vk_resource_buffer_memory, TessellationControlBufferMemoryOffset + (sizeof(ResourceBuffer) * m_tessellation_control_resource_update_count), sizeof(ResourceBuffer), 0, std::addressof(memory_address));
            DD_ASSERT(result0 == VK_SUCCESS);

            ::memcpy(memory_address, std::addressof(m_resource_buffer_per_stage_array[ShaderStage_TessellationControl]), sizeof(ResourceBuffer));

            ::pfn_vkUnmapMemory(GetGlobalContext()->GetDevice(), m_vk_resource_buffer_memory);

            m_tessellation_control_resource_update_count += 1;
        }
        if (m_need_geometry_resource_update == true) {

            void *memory_address = nullptr;
            const u32 result0 = ::pfn_vkMapMemory(GetGlobalContext()->GetDevice(), m_vk_resource_buffer_memory, GeometryBufferMemoryOffset + (sizeof(ResourceBuffer) * m_geometry_resource_update_count), sizeof(ResourceBuffer), 0, std::addressof(memory_address));
            DD_ASSERT(result0 == VK_SUCCESS);

            ::memcpy(memory_address, std::addressof(m_resource_buffer_per_stage_array[ShaderStage_Geometry]), sizeof(ResourceBuffer));

            ::pfn_vkUnmapMemory(GetGlobalContext()->GetDevice(), m_vk_resource_buffer_memory);

            m_geometry_resource_update_count += 1;
        }
        if (m_need_fragment_resource_update == true) {

            void *memory_address = nullptr;
            const u32 result0 = ::pfn_vkMapMemory(GetGlobalContext()->GetDevice(), m_vk_resource_buffer_memory, FragmentBufferMemoryOffset + (sizeof(ResourceBuffer) * m_fragment_resource_update_count), sizeof(ResourceBuffer), 0, std::addressof(memory_address));
            DD_ASSERT(result0 == VK_SUCCESS);

            ::memcpy(memory_address, std::addressof(m_resource_buffer_per_stage_array[ShaderStage_Fragment]), sizeof(ResourceBuffer));

            ::pfn_vkUnmapMemory(GetGlobalContext()->GetDevice(), m_vk_resource_buffer_memory);

            m_fragment_resource_update_count += 1;
        }
        if (m_need_compute_resource_update == true) {

            void *memory_address = nullptr;
            const u32 result0 = ::pfn_vkMapMemory(GetGlobalContext()->GetDevice(), m_vk_resource_buffer_memory, ComputeBufferMemoryOffset + (sizeof(ResourceBuffer) * m_compute_resource_update_count), sizeof(ResourceBuffer), 0, std::addressof(memory_address));
            DD_ASSERT(result0 == VK_SUCCESS);

            ::memcpy(memory_address, std::addressof(m_resource_buffer_per_stage_array[ShaderStage_Compute]), sizeof(ResourceBuffer));

            ::pfn_vkUnmapMemory(GetGlobalContext()->GetDevice(), m_vk_resource_buffer_memory);

            m_compute_resource_update_count += 1;
        }
    }

    void CommandBuffer::PushResourceBufferIndices() {
        if (m_vertex_resource_update_count != 0) {
            const u32 resource_index = m_vertex_resource_update_count - 1;
            ::pfn_vkCmdPushConstants(m_vk_command_buffer, GetGlobalContext()->GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(u32), std::addressof(resource_index));
        }
        if (m_tessellation_evaluation_resource_update_count != 0) {
            const u32 resource_index = m_tessellation_evaluation_resource_update_count - 1;
            ::pfn_vkCmdPushConstants(m_vk_command_buffer, GetGlobalContext()->GetPipelineLayout(), VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, sizeof(u32), sizeof(u32), std::addressof(resource_index));
        }
        if (m_tessellation_control_resource_update_count != 0) {
            const u32 resource_index = m_tessellation_control_resource_update_count - 1;
            ::pfn_vkCmdPushConstants(m_vk_command_buffer, GetGlobalContext()->GetPipelineLayout(), VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, sizeof(u32) * 2, sizeof(u32), std::addressof(resource_index));
        }
        if (m_geometry_resource_update_count != 0) {
            const u32 resource_index = m_geometry_resource_update_count - 1;
            ::pfn_vkCmdPushConstants(m_vk_command_buffer, GetGlobalContext()->GetPipelineLayout(), VK_SHADER_STAGE_GEOMETRY_BIT, sizeof(u32) * 3, sizeof(u32), std::addressof(resource_index));
        }
        if (m_fragment_resource_update_count != 0) {
            const u32 resource_index = m_fragment_resource_update_count - 1;
            ::pfn_vkCmdPushConstants(m_vk_command_buffer, GetGlobalContext()->GetPipelineLayout(), VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(u32) * 4, sizeof(u32), std::addressof(resource_index));
        }
        if (m_compute_resource_update_count != 0) {
            const u32 resource_index = m_compute_resource_update_count - 1;
            ::pfn_vkCmdPushConstants(m_vk_command_buffer, GetGlobalContext()->GetPipelineLayout(), VK_SHADER_STAGE_COMPUTE_BIT, sizeof(u32) * 5, sizeof(u32), std::addressof(resource_index));
        }
    }

    /* Public */

    void CommandBuffer::Initialize(const Context *context) {

        /* Allocate Commmand Buffer */
        const VkCommandBufferAllocateInfo allocate_info = {
            .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool        = context->GetGraphicsCommandPool(),
            .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1
        };

        const u32 result0 = ::pfn_vkAllocateCommandBuffers(context->GetDevice(), std::addressof(allocate_info), std::addressof(m_vk_command_buffer));
        DD_ASSERT(result0 == VK_SUCCESS);

        /* Create resource buffer descriptor pool */
        const VkDescriptorPoolSize pool_size = {
            .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = 6
        };

        const VkDescriptorPoolCreateInfo buffer_pool_info = {
            .sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .maxSets       = 1,
            .poolSizeCount = 1,
            .pPoolSizes    = std::addressof(pool_size)
        };

        const u32 result1 = ::pfn_vkCreateDescriptorPool(context->GetDevice(), std::addressof(buffer_pool_info), nullptr, std::addressof(m_vk_resource_buffer_descriptor_pool));
        DD_ASSERT(result1 == VK_SUCCESS);

        /* Create resource buffer descriptor set */
        VkDescriptorSetLayout vk_buffer_descriptor_set_layout = context->GetResourceBufferDescriptorSetLayout();
        const VkDescriptorSetAllocateInfo buffer_set_info = {
            .sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .descriptorPool     = m_vk_resource_buffer_descriptor_pool,
            .descriptorSetCount = 1,
            .pSetLayouts        = std::addressof(vk_buffer_descriptor_set_layout)
        };

        const u32 result2 = ::pfn_vkAllocateDescriptorSets(context->GetDevice(), std::addressof(buffer_set_info), std::addressof(m_vk_resource_buffer_descriptor_set));
        DD_ASSERT(result2 == VK_SUCCESS);

        /* Allocate resource buffer memory */
        const s32 device_memory_type = context->FindMemoryHeapIndex(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        DD_ASSERT(device_memory_type != -1);

        const VkMemoryAllocateInfo device_allocate_info =  {
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .allocationSize = TargetResourceBufferMemorySize,
            .memoryTypeIndex = static_cast<u32>(device_memory_type)
        };
        const u32 result3 = ::pfn_vkAllocateMemory(context->GetDevice(), std::addressof(device_allocate_info), nullptr, std::addressof(m_vk_resource_buffer_memory));
        DD_ASSERT(result3 == VK_SUCCESS);

        /* Create resource buffers for each stage */
        const u32 queue_family_index = context->GetGraphicsQueueFamilyIndex();
        const VkBufferCreateInfo buffer_create_info = {
            .sType                 = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size                  = TargetResourceBufferPerStageSize,
            .usage                 = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            .sharingMode           = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 1,
            .pQueueFamilyIndices   = std::addressof(queue_family_index)
        };

        const u32 result4 = ::pfn_vkCreateBuffer(context->GetDevice(), std::addressof(buffer_create_info), nullptr, std::addressof(m_vk_resource_buffer_per_stage_array[ShaderStage_Vertex]));
        DD_ASSERT(result4 == VK_SUCCESS);

        const u32 result5 = ::pfn_vkCreateBuffer(context->GetDevice(), std::addressof(buffer_create_info), nullptr, std::addressof(m_vk_resource_buffer_per_stage_array[ShaderStage_TessellationEvaluation]));
        DD_ASSERT(result5 == VK_SUCCESS);

        const u32 result6 = ::pfn_vkCreateBuffer(context->GetDevice(), std::addressof(buffer_create_info), nullptr, std::addressof(m_vk_resource_buffer_per_stage_array[ShaderStage_TessellationControl]));
        DD_ASSERT(result6 == VK_SUCCESS);

        const u32 result7 = ::pfn_vkCreateBuffer(context->GetDevice(), std::addressof(buffer_create_info), nullptr, std::addressof(m_vk_resource_buffer_per_stage_array[ShaderStage_Geometry]));
        DD_ASSERT(result7 == VK_SUCCESS);

        const u32 result8 = ::pfn_vkCreateBuffer(context->GetDevice(), std::addressof(buffer_create_info), nullptr, std::addressof(m_vk_resource_buffer_per_stage_array[ShaderStage_Fragment]));
        DD_ASSERT(result8 == VK_SUCCESS);

        const u32 result9 = ::pfn_vkCreateBuffer(context->GetDevice(), std::addressof(buffer_create_info), nullptr, std::addressof(m_vk_resource_buffer_per_stage_array[ShaderStage_Compute]));
        DD_ASSERT(result9 == VK_SUCCESS);

        /* Bind resource buffer memory to resource buffers */
        const u32 result10 = ::pfn_vkBindBufferMemory(context->GetDevice(), m_vk_resource_buffer_per_stage_array[ShaderStage_Vertex], m_vk_resource_buffer_memory, VertexBufferMemoryOffset);
        DD_ASSERT(result10 == VK_SUCCESS);

        const u32 result11 = ::pfn_vkBindBufferMemory(context->GetDevice(), m_vk_resource_buffer_per_stage_array[ShaderStage_TessellationEvaluation], m_vk_resource_buffer_memory, TessellationEvaluationBufferMemoryOffset);
        DD_ASSERT(result11 == VK_SUCCESS);

        const u32 result12 = ::pfn_vkBindBufferMemory(context->GetDevice(), m_vk_resource_buffer_per_stage_array[ShaderStage_TessellationControl], m_vk_resource_buffer_memory, TessellationControlBufferMemoryOffset);
        DD_ASSERT(result12 == VK_SUCCESS);

        const u32 result13 = ::pfn_vkBindBufferMemory(context->GetDevice(), m_vk_resource_buffer_per_stage_array[ShaderStage_Geometry], m_vk_resource_buffer_memory, GeometryBufferMemoryOffset);
        DD_ASSERT(result13 == VK_SUCCESS);

        const u32 result14 = ::pfn_vkBindBufferMemory(context->GetDevice(), m_vk_resource_buffer_per_stage_array[ShaderStage_Fragment], m_vk_resource_buffer_memory, FragmentBufferMemoryOffset);
        DD_ASSERT(result14 == VK_SUCCESS);

        const u32 result15 = ::pfn_vkBindBufferMemory(context->GetDevice(), m_vk_resource_buffer_per_stage_array[ShaderStage_Compute], m_vk_resource_buffer_memory, ComputeBufferMemoryOffset);
        DD_ASSERT(result15 == VK_SUCCESS);

        /* Update resource buffer descriptor */
        const VkDescriptorBufferInfo descriptor_buffer_info[Context::TargetShaderStages] = {
            {
                .buffer = m_vk_resource_buffer_per_stage_array[ShaderStage_Vertex],
                .offset = 0,
                .range  = VK_WHOLE_SIZE
            },
            {
                .buffer = m_vk_resource_buffer_per_stage_array[ShaderStage_TessellationEvaluation],
                .offset = 0,
                .range  = VK_WHOLE_SIZE
            },
            {
                .buffer = m_vk_resource_buffer_per_stage_array[ShaderStage_TessellationControl],
                .offset = 0,
                .range  = VK_WHOLE_SIZE
            },
            {
                .buffer = m_vk_resource_buffer_per_stage_array[ShaderStage_Geometry],
                .offset = 0,
                .range  = VK_WHOLE_SIZE
            },
            {
                .buffer = m_vk_resource_buffer_per_stage_array[ShaderStage_Fragment],
                .offset = 0,
                .range  = VK_WHOLE_SIZE
            },
            {
                .buffer = m_vk_resource_buffer_per_stage_array[ShaderStage_Compute],
                .offset = 0,
                .range  = VK_WHOLE_SIZE
            }
        };
        const VkWriteDescriptorSet write_set[Context::TargetShaderStages] = {
            {
                .sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet          = m_vk_resource_buffer_descriptor_set,
                .dstBinding      = Context::TargetVertexResourceBufferDescriptorBinding,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .pBufferInfo     = std::addressof(descriptor_buffer_info[ShaderStage_Vertex]),
            },
            {
                .sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet          = m_vk_resource_buffer_descriptor_set,
                .dstBinding      = Context::TargetTessellationEvaluationResourceBufferDescriptorBinding,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .pBufferInfo     = std::addressof(descriptor_buffer_info[ShaderStage_TessellationEvaluation]),
            },
            {
                .sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet          = m_vk_resource_buffer_descriptor_set,
                .dstBinding      = Context::TargetTessellationControlResourceBufferDescriptorBinding,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .pBufferInfo     = std::addressof(descriptor_buffer_info[ShaderStage_TessellationControl]),
            },
            {
                .sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet          = m_vk_resource_buffer_descriptor_set,
                .dstBinding      = Context::TargetGeometryResourceBufferDescriptorBinding,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .pBufferInfo     = std::addressof(descriptor_buffer_info[ShaderStage_Geometry]),
            },
            {
                .sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet          = m_vk_resource_buffer_descriptor_set,
                .dstBinding      = Context::TargetFragmentResourceBufferDescriptorBinding,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .pBufferInfo     = std::addressof(descriptor_buffer_info[ShaderStage_Fragment]),
            },
            {
                .sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet          = m_vk_resource_buffer_descriptor_set,
                .dstBinding      = Context::TargetComputeResourceBufferDescriptorBinding,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .pBufferInfo     = std::addressof(descriptor_buffer_info[ShaderStage_Compute]),
            }
        };

        ::pfn_vkUpdateDescriptorSets(context->GetDevice(), sizeof(write_set) / sizeof(VkWriteDescriptorSet), write_set, 0, nullptr);
    }

    void CommandBuffer::Finalize(const Context *context) {

        ::pfn_vkFreeCommandBuffers(context->GetDevice(), context->GetGraphicsCommandPool(), 1, std::addressof(m_vk_command_buffer));

        for (u32 i = 0; i < Context::TargetShaderStages; ++i) {
            ::pfn_vkDestroyBuffer(context->GetDevice(), m_vk_resource_buffer_per_stage_array[i], nullptr);
        }
        ::pfn_vkDestroyDescriptorPool(context->GetDevice(), m_vk_resource_buffer_descriptor_pool, nullptr);
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

        /* Bind Resource buffer */
        ::pfn_vkCmdBindDescriptorSets(m_vk_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, GetGlobalContext()->GetPipelineLayout(), 2, 1, std::addressof(m_vk_resource_buffer_descriptor_set), 0, nullptr);

        m_resource_buffer_per_stage_array[ShaderStage_Vertex].SetDefaults();
        m_resource_buffer_per_stage_array[ShaderStage_TessellationControl].SetDefaults();
        m_resource_buffer_per_stage_array[ShaderStage_TessellationEvaluation].SetDefaults();
        m_resource_buffer_per_stage_array[ShaderStage_Geometry].SetDefaults();
        m_resource_buffer_per_stage_array[ShaderStage_Fragment].SetDefaults();
        m_resource_buffer_per_stage_array[ShaderStage_Compute].SetDefaults();
        m_vertex_resource_update_count = 0;
        m_tessellation_evaluation_resource_update_count = 0;
        m_tessellation_control_resource_update_count = 0;
        m_geometry_resource_update_count = 0;
        m_fragment_resource_update_count = 0;
        m_compute_resource_update_count = 0;
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

    void CommandBuffer::SetFastClearColor(const VkClearColorValue& color) {
        m_vk_clear_color.float32[0] = color.float32[0];
        m_vk_clear_color.float32[1] = color.float32[1];
        m_vk_clear_color.float32[2] = color.float32[2];
        m_vk_clear_color.float32[3] = color.float32[3];
        m_fast_clear = true;
    }

    void CommandBuffer::Draw(VkPrimitiveTopology vk_primitive_topology, u32 vertex_count, u32 base_vertex) {

        this->UpdateResourceBufferIfNecessary();
        this->BeginRenderingIfNotRendering();

        /* Set resource indices */
        this->PushResourceBufferIndices();

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

        /* Set resource indices */
        this->PushResourceBufferIndices();

        /* Draw */
        ::pfn_vkCmdSetPrimitiveTopology(m_vk_command_buffer, vk_primitive_topology);
        ::pfn_vkCmdDrawIndexed(m_vk_command_buffer, index_count, 1, base_index, 0, 0);
    }

    void CommandBuffer::DrawInstanced(VkPrimitiveTopology vk_primitive_topology, u32 vertex_count, u32 base_vertex, u32 instance_count, u32 base_instance) {

        this->UpdateResourceBufferIfNecessary();
        this->BeginRenderingIfNotRendering();

        /* Set resource indices */
        this->PushResourceBufferIndices();

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

        /* Set resource indices */
        this->PushResourceBufferIndices();

        /* Draw */
        ::pfn_vkCmdSetPrimitiveTopology(m_vk_command_buffer, vk_primitive_topology);
        ::pfn_vkCmdDrawIndexed(m_vk_command_buffer, index_count, instance_count, base_index, 0, base_instance);
    }
    
    

    void CommandBuffer::SetPipeline(Pipeline *pipeline) {
        ::pfn_vkCmdBindPipeline(m_vk_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->GetPipeline());
    }

    void CommandBuffer::SetPipelineState(const PipelineCmdState *pipeline_state) {
        this->SetColorBlendState(std::addressof(pipeline_state->color_blend_state));
        this->SetDepthStencilState(std::addressof(pipeline_state->depth_stencil_state));
        this->SetRasterizerState(std::addressof(pipeline_state->rasterizer_state));
        this->SetVertexState(std::addressof(pipeline_state->vertex_state));
    }

    void CommandBuffer::SetColorBlendState(const ColorBlendCmdState *color_blend_state) {
        pfn_vkCmdSetLogicOpEXT(m_vk_command_buffer, color_blend_state->vk_logic_op);
        ::pfn_vkCmdSetBlendConstants(m_vk_command_buffer, color_blend_state->blend_constants);
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
