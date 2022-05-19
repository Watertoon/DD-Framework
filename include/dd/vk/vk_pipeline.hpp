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

    struct RasterizerPipelineState {
        bool depth_clamp_enable;
        u32  vk_polygon_mode;

        constexpr void SetDefaults() {
            depth_clamp_enable = false;
            vk_polygon_mode    = VK_POLYGON_MODE_FILL;
        }
    };

    struct ColorBlendPipelineState {
        u32 color_blend_count;
        VkPipelineColorBlendAttachmentState vk_color_blend_attachment_states[Context::TargetColorAttachmentCount];

        constexpr void SetDefaults() {
            for (u32 i = 0; i < Context::TargetColorAttachmentCount; ++i) {
                vk_color_blend_attachment_states[i].blendEnable = false;
                vk_color_blend_attachment_states[i].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
            }
            color_blend_count = 1;
        }
    };

    struct PipelineInfo {
        Shader                    *shader;
        RasterizerPipelineState   rasterization_state;
        ColorBlendPipelineState   color_blend_state;
        VkPrimitiveTopology       vk_primitive_topology;

        constexpr void SetDefaults() {
            shader = nullptr;
            rasterization_state.SetDefaults();
            color_blend_state.SetDefaults();
            vk_primitive_topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        }
    };

    class Pipeline {
        private:
            VkPipeline            m_vk_pipeline;
            VkDescriptorSetLayout m_vk_descriptor_set_layout;
            VkPipelineLayout      m_vk_pipeline_layout;
        public:
            constexpr Pipeline() {/*...*/}

            void Initialize(const Context *context, const PipelineInfo *pipeline_info) {
                
                /* pNext Pipeline State */
                VkFormat color_formats[Context::TargetColorAttachmentCount] = {};
                for (u32 i = 0; i < pipeline_info->color_blend_state.color_blend_count; ++i) {
                    color_formats[i] = Context::TargetSurfaceFormat.format;
                }
                const VkPipelineRenderingCreateInfo rendering_info = {
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
                    .colorAttachmentCount = pipeline_info->color_blend_state.color_blend_count,
                    .pColorAttachmentFormats = color_formats
                };

                /* Pipeline state */
                VkPipelineShaderStageCreateInfo shader_create_info[Context::TargetShaderStages] = {};
        
                u32 stage_count = 0;
                const VkShaderModule vertex_module = pipeline_info->shader->GetVertexModule();
                if (vertex_module != 0) {
                    shader_create_info[stage_count].sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                    shader_create_info[stage_count].stage  = VK_SHADER_STAGE_VERTEX_BIT;
                    shader_create_info[stage_count].module = vertex_module;
                    shader_create_info[stage_count].pName   = "main";
                    ++stage_count;
                }
                const VkShaderModule tesselation_control_module = pipeline_info->shader->GetTessellationControlModule();
                if (tesselation_control_module != 0) {
                    shader_create_info[stage_count].sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                    shader_create_info[stage_count].stage  = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
                    shader_create_info[stage_count].module = tesselation_control_module;
                    shader_create_info[stage_count].pName   = "main";
                    ++stage_count;
                }
                const VkShaderModule tesselation_evaluation_module = pipeline_info->shader->GetTessellationEvaluationModule();
                if (tesselation_evaluation_module != 0) {
                    shader_create_info[stage_count].sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                    shader_create_info[stage_count].stage  = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
                    shader_create_info[stage_count].module = tesselation_evaluation_module;
                    shader_create_info[stage_count].pName   = "main";
                    ++stage_count;
                }
                const VkShaderModule geometry_module = pipeline_info->shader->GetGeometryModule();
                if (geometry_module != 0) {
                    shader_create_info[stage_count].sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                    shader_create_info[stage_count].stage  = VK_SHADER_STAGE_GEOMETRY_BIT;
                    shader_create_info[stage_count].module = geometry_module;
                    shader_create_info[stage_count].pName   = "main";
                    ++stage_count;
                }
                const VkShaderModule fragment_module = pipeline_info->shader->GetFragmentModule();
                if (fragment_module != 0) {
                    shader_create_info[stage_count].sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                    shader_create_info[stage_count].stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
                    shader_create_info[stage_count].module = fragment_module;
                    shader_create_info[stage_count].pName   = "main";
                    ++stage_count;
                }
                const VkShaderModule compute_module = pipeline_info->shader->GetComputeModule();
                if (compute_module != 0) {
                    shader_create_info[stage_count].sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                    shader_create_info[stage_count].stage  = VK_SHADER_STAGE_COMPUTE_BIT;
                    shader_create_info[stage_count].module = compute_module;
                    shader_create_info[stage_count].pName   = "main";
                    ++stage_count;
                }

                const VkPipelineInputAssemblyStateCreateInfo input_assembly_state = {
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
                    .topology = pipeline_info->vk_primitive_topology
                };

                const VkPipelineViewportStateCreateInfo viewport_scissor_state = {
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO
                };

                const VkPipelineRasterizationStateCreateInfo rasterization_state = {
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
                    .depthClampEnable = pipeline_info->rasterization_state.depth_clamp_enable,
                    .polygonMode = static_cast<VkPolygonMode>(pipeline_info->rasterization_state.vk_polygon_mode)
                };

                const VkPipelineMultisampleStateCreateInfo multisample_state = {
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
                    .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
                    .sampleShadingEnable = VK_FALSE
                };

                const VkPipelineDepthStencilStateCreateInfo depth_stencil_state = {
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO
                };

                const VkPipelineColorBlendStateCreateInfo color_blend_info = {
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
                    .logicOpEnable = VK_TRUE,
                    .attachmentCount = pipeline_info->color_blend_state.color_blend_count,
                    .pAttachments = pipeline_info->color_blend_state.vk_color_blend_attachment_states
                };

                const VkDynamicState dynamic_state_array[] = {
                    VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT,
                    VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT,
                    VK_DYNAMIC_STATE_LINE_WIDTH,
                    VK_DYNAMIC_STATE_DEPTH_BIAS,
                    VK_DYNAMIC_STATE_BLEND_CONSTANTS,
                    VK_DYNAMIC_STATE_DEPTH_BOUNDS,
                    VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK,
                    VK_DYNAMIC_STATE_STENCIL_WRITE_MASK,
                    VK_DYNAMIC_STATE_STENCIL_REFERENCE,
                    VK_DYNAMIC_STATE_CULL_MODE,
                    VK_DYNAMIC_STATE_FRONT_FACE,
                    VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY,
                    VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE,
                    VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE,
                    VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE,
                    VK_DYNAMIC_STATE_DEPTH_COMPARE_OP,
                    VK_DYNAMIC_STATE_DEPTH_BOUNDS_TEST_ENABLE,
                    VK_DYNAMIC_STATE_STENCIL_TEST_ENABLE,
                    VK_DYNAMIC_STATE_STENCIL_OP,
                    VK_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE,
                    VK_DYNAMIC_STATE_DEPTH_BIAS_ENABLE,
                    VK_DYNAMIC_STATE_PRIMITIVE_RESTART_ENABLE,

                    VK_DYNAMIC_STATE_VERTEX_INPUT_EXT,

                    VK_DYNAMIC_STATE_LOGIC_OP_EXT
                };
                const VkPipelineDynamicStateCreateInfo dynamic_state = {
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
                    .dynamicStateCount = sizeof(dynamic_state_array) / sizeof(VkDynamicState),
                    .pDynamicStates = dynamic_state_array
                };

                /* Create Pipeline */
                const VkGraphicsPipelineCreateInfo graphics_pipeline_info = {
                    .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
                    .pNext = std::addressof(rendering_info),
                    .stageCount = stage_count,
                    .pStages = shader_create_info,
                    .pInputAssemblyState = std::addressof(input_assembly_state),
                    .pViewportState = std::addressof(viewport_scissor_state),
                    .pRasterizationState = std::addressof(rasterization_state),
                    .pMultisampleState = std::addressof(multisample_state),
                    .pDepthStencilState = std::addressof(depth_stencil_state),
                    .pColorBlendState = std::addressof(color_blend_info),
                    .pDynamicState = std::addressof(dynamic_state),
                    .layout = context->GetPipelineLayout()
                };

                const u32 result2 = ::pfn_vkCreateGraphicsPipelines(context->GetDevice(), 0, 1, std::addressof(graphics_pipeline_info), nullptr, std::addressof(m_vk_pipeline));
                DD_ASSERT(result2 == VK_SUCCESS);
            }

            void Finalize(const Context *context) {
                ::pfn_vkDestroyPipeline(context->GetDevice(), m_vk_pipeline, nullptr);
            }

            constexpr ALWAYS_INLINE VkPipeline GetPipeline() { return m_vk_pipeline; } 
    };
}
