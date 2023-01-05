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
#pragma once

namespace dd::vk {

    enum ShaderStage {
        ShaderStage_Vertex                 = 0,
        ShaderStage_TessellationEvaluation = 1,
        ShaderStage_TessellationControl    = 2,
        ShaderStage_Geometry               = 3,
        ShaderStage_Fragment               = 4,
        ShaderStage_Compute                = 5,
    };
    
    enum PipelineType {
        PipelineType_Invalid,
        PipelineType_Primitive,
        PipelineType_Mesh,
        PipelineType_RayTracing,
        PipelineType_Compute,
    };

    struct PrimitiveShaderInfo {
        u32     color_attachment_count;
        size_t  vertex_code_size;
        u32    *vertex_code;
        size_t  tessellation_control_code_size;
        u32    *tessellation_control_code;
        size_t  tessellation_evaluation_code_size;
        u32    *tessellation_evaluation_code;
        size_t  geometry_code_size;
        u32    *geometry_code;
        size_t  fragment_code_size;
        u32    *fragment_code;
    };

    struct ComputeShaderInfo {
        size_t  compute_code_size;
        u32    *compute_code;
    };

    struct MeshShaderInfo {
        u32     color_attachment_count;
        size_t  task_code_size;
        u32    *task_code;
        size_t  mesh_code_size;
        u32    *mesh_code;
        size_t  fragment_code_size;
        u32    *fragment_code;
    };
    
    struct RayTracingShaderInfo {
        u32     color_attachment_count;
        size_t  ray_generation_code_size;
        u32    *ray_generation_code;
        size_t  intersection_code_size;
        u32    *intersection_code;
        size_t  any_hit_code_size;
        u32    *any_hit_code;
        size_t  closest_hit_code_size;
        u32    *closest_hit_code;
        size_t  miss_code_size;
        u32    *miss_code;
        size_t  callable_code_size;
        u32    *callable_code;
    };

    class Shader {
        private:
            VkPipeline m_vk_pipeline;
            u32        m_pipeline_type;
        public:
            constexpr Shader() : m_vk_pipeline(0), m_pipeline_type(0) {/*...*/}

            void Initialize(const Context *context, const PrimitiveShaderInfo *shader_info) {

                /* Create shader modules */
                VkShaderModule vk_vertex_module = 0;
                VkShaderModule vk_tessellation_control_module = 0;
                VkShaderModule vk_tessellation_evaluation_module = 0;
                VkShaderModule vk_geometry_module = 0;
                VkShaderModule vk_fragment_module = 0;
                if (shader_info->vertex_code != nullptr) {
                    const VkShaderModuleCreateInfo shader_module_info = {
                        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
                        .codeSize = shader_info->vertex_code_size,
                        .pCode = shader_info->vertex_code
                    };
                    const u32 result = ::pfn_vkCreateShaderModule(context->GetDevice(), std::addressof(shader_module_info), nullptr, std::addressof(vk_vertex_module));
                    DD_ASSERT(result == VK_SUCCESS);
                }
                if (shader_info->tessellation_control_code != nullptr) {
                    const VkShaderModuleCreateInfo shader_module_info = {
                        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
                        .codeSize = shader_info->tessellation_control_code_size,
                        .pCode = shader_info->tessellation_control_code
                    };
                    const u32 result = ::pfn_vkCreateShaderModule(context->GetDevice(), std::addressof(shader_module_info), nullptr, std::addressof(vk_tessellation_control_module));
                    DD_ASSERT(result == VK_SUCCESS);
                }
                if (shader_info->tessellation_evaluation_code != nullptr) {
                    const VkShaderModuleCreateInfo shader_module_info = {
                        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
                        .codeSize = shader_info->tessellation_evaluation_code_size,
                        .pCode = shader_info->tessellation_evaluation_code
                    };
                    const u32 result = ::pfn_vkCreateShaderModule(context->GetDevice(), std::addressof(shader_module_info), nullptr, std::addressof(vk_tessellation_evaluation_module));
                    DD_ASSERT(result == VK_SUCCESS);
                }
                if (shader_info->geometry_code != nullptr) {
                    const VkShaderModuleCreateInfo shader_module_info = {
                        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
                        .codeSize = shader_info->geometry_code_size,
                        .pCode = shader_info->geometry_code
                    };
                    const u32 result = ::pfn_vkCreateShaderModule(context->GetDevice(), std::addressof(shader_module_info), nullptr, std::addressof(vk_geometry_module));
                    DD_ASSERT(result == VK_SUCCESS);
                }
                if (shader_info->fragment_code != nullptr) {
                    const VkShaderModuleCreateInfo shader_module_info = {
                        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
                        .codeSize = shader_info->fragment_code_size,
                        .pCode = shader_info->fragment_code
                    };
                    const u32 result = ::pfn_vkCreateShaderModule(context->GetDevice(), std::addressof(shader_module_info), nullptr, std::addressof(vk_fragment_module));
                    DD_ASSERT(result == VK_SUCCESS);
                }

                /* Rendering info */
                VkFormat color_formats[Context::TargetColorAttachmentCount] = {};
                for (u32 i = 0; i < Context::TargetColorAttachmentCount; ++i) {
                    color_formats[i] = Context::TargetSurfaceFormat.format;
                }
                const VkPipelineRenderingCreateInfo rendering_info = {
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
                    .colorAttachmentCount = shader_info->color_attachment_count,
                    .pColorAttachmentFormats = color_formats,
                    .depthAttachmentFormat = Context::TargetDepthStencilFormat,
                    .stencilAttachmentFormat = Context::TargetDepthStencilFormat
                };

                /* Pipeline state */
                VkPipelineShaderStageCreateInfo shader_create_info[Context::TargetShaderStages - 1] = {};
                u32 stage_count = 0;
                if (vk_vertex_module != 0) {
                    shader_create_info[stage_count].sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                    shader_create_info[stage_count].stage  = VK_SHADER_STAGE_VERTEX_BIT;
                    shader_create_info[stage_count].module = vk_vertex_module;
                    shader_create_info[stage_count].pName   = "main";
                    ++stage_count;
                }
                if (vk_tessellation_control_module != 0) {
                    shader_create_info[stage_count].sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                    shader_create_info[stage_count].stage  = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
                    shader_create_info[stage_count].module = vk_tessellation_control_module;
                    shader_create_info[stage_count].pName   = "main";
                    ++stage_count;
                }
                if (vk_tessellation_evaluation_module != 0) {
                    shader_create_info[stage_count].sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                    shader_create_info[stage_count].stage  = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
                    shader_create_info[stage_count].module = vk_tessellation_evaluation_module;
                    shader_create_info[stage_count].pName   = "main";
                    ++stage_count;
                }
                if (vk_geometry_module != 0) {
                    shader_create_info[stage_count].sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                    shader_create_info[stage_count].stage  = VK_SHADER_STAGE_GEOMETRY_BIT;
                    shader_create_info[stage_count].module = vk_geometry_module;
                    shader_create_info[stage_count].pName   = "main";
                    ++stage_count;
                }
                if (vk_fragment_module != 0) {
                    shader_create_info[stage_count].sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                    shader_create_info[stage_count].stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
                    shader_create_info[stage_count].module = vk_fragment_module;
                    shader_create_info[stage_count].pName   = "main";
                    ++stage_count;
                }

                const VkPipelineInputAssemblyStateCreateInfo input_assembly_state = {
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
                    .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST
                };

                const VkPipelineViewportStateCreateInfo viewport_scissor_state = {
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO
                };

                const VkPipelineRasterizationStateCreateInfo rasterization_state = {
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO
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
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO
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

                    /* Ext vertex input */
                    VK_DYNAMIC_STATE_VERTEX_INPUT_EXT,

                    /* Ext logic op */
                    VK_DYNAMIC_STATE_LOGIC_OP_EXT,

                    /* Ext 3 */
                    //VK_DYNAMIC_STATE_COLOR_BLEND_ADVANCED_EXT,
                    VK_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT,
                    VK_DYNAMIC_STATE_COLOR_BLEND_EQUATION_EXT,
                    VK_DYNAMIC_STATE_COLOR_WRITE_MASK_EXT,
                    //VK_DYNAMIC_STATE_CONSERVATIVE_RASTERIZATION_MODE_EXT,
                    VK_DYNAMIC_STATE_DEPTH_CLAMP_ENABLE_EXT,
                    VK_DYNAMIC_STATE_LOGIC_OP_ENABLE_EXT,
                    VK_DYNAMIC_STATE_POLYGON_MODE_EXT,
                    //VK_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT,
                    //VK_DYNAMIC_STATE_SAMPLE_MASK_EXT,
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
                m_pipeline_type = PipelineType_Primitive;

                /* Cleanup shader modules */
                if (vk_vertex_module != 0) {
                    ::pfn_vkDestroyShaderModule(context->GetDevice(), vk_vertex_module, nullptr);
                }
                if (vk_tessellation_control_module != 0) {
                    ::pfn_vkDestroyShaderModule(context->GetDevice(), vk_tessellation_control_module, nullptr);
                }
                if (vk_tessellation_evaluation_module != 0) {
                    ::pfn_vkDestroyShaderModule(context->GetDevice(), vk_tessellation_evaluation_module, nullptr);
                }
                if (vk_geometry_module != 0) {
                    ::pfn_vkDestroyShaderModule(context->GetDevice(), vk_geometry_module, nullptr);
                }
                if (vk_fragment_module != 0) {
                    ::pfn_vkDestroyShaderModule(context->GetDevice(), vk_fragment_module, nullptr);
                }
            }

            void Initialize(const Context *context, const ComputeShaderInfo *shader_info) {
                
                /* Create compute shader module*/
                VkShaderModule vk_compute_module = 0;
                if (shader_info->compute_code != nullptr) {
                    const VkShaderModuleCreateInfo shader_module_info = {
                        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
                        .codeSize = shader_info->compute_code_size,
                        .pCode = shader_info->compute_code
                    };
                    const u32 result0 = ::pfn_vkCreateShaderModule(context->GetDevice(), std::addressof(shader_module_info), nullptr, std::addressof(vk_compute_module));
                    DD_ASSERT(result0 == VK_SUCCESS);
                }

                /* Set pipeline info */
                const VkComputePipelineCreateInfo pipeline_info = {
                    .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
                    .stage = {
                        .sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                        .stage  = VK_SHADER_STAGE_COMPUTE_BIT,
                        .module = vk_compute_module,
                        .pName  = "main"
                    },
                    .layout = context->GetPipelineLayout()
                };

                /* Create compute pipeline */
                const u32 result1 = ::pfn_vkCreateComputePipelines(context->GetDevice(), 0, 1, std::addressof(pipeline_info), nullptr, std::addressof(m_vk_pipeline));
                DD_ASSERT(result1 == VK_SUCCESS);
                m_pipeline_type = PipelineType_Compute;

                /* Cleanup shader module */
                if (vk_compute_module != 0) {
                    ::pfn_vkDestroyShaderModule(context->GetDevice(), vk_compute_module, nullptr);
                }
            }

            void Initialize(const Context *context, const MeshShaderInfo *shader_info) {

                /* Create shader modules */
                VkShaderModule vk_task_module = 0;
                VkShaderModule vk_mesh_module = 0;
                VkShaderModule vk_fragment_module = 0;
                if (shader_info->task_code != nullptr) {
                    const VkShaderModuleCreateInfo shader_module_info = {
                        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
                        .codeSize = shader_info->task_code_size,
                        .pCode = shader_info->task_code
                    };
                    const u32 result = ::pfn_vkCreateShaderModule(context->GetDevice(), std::addressof(shader_module_info), nullptr, std::addressof(vk_task_module));
                    DD_ASSERT(result == VK_SUCCESS);
                }
                if (shader_info->mesh_code != nullptr) {
                    const VkShaderModuleCreateInfo shader_module_info = {
                        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
                        .codeSize = shader_info->mesh_code_size,
                        .pCode = shader_info->mesh_code
                    };
                    const u32 result = ::pfn_vkCreateShaderModule(context->GetDevice(), std::addressof(shader_module_info), nullptr, std::addressof(vk_mesh_module));
                    DD_ASSERT(result == VK_SUCCESS);
                }
                if (shader_info->fragment_code != nullptr) {
                    const VkShaderModuleCreateInfo shader_module_info = {
                        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
                        .codeSize = shader_info->fragment_code_size,
                        .pCode = shader_info->fragment_code
                    };
                    const u32 result = ::pfn_vkCreateShaderModule(context->GetDevice(), std::addressof(shader_module_info), nullptr, std::addressof(vk_fragment_module));
                    DD_ASSERT(result == VK_SUCCESS);
                }

                /* Rendering info */
                VkFormat color_formats[Context::TargetColorAttachmentCount] = {};
                for (u32 i = 0; i < Context::TargetColorAttachmentCount; ++i) {
                    color_formats[i] = Context::TargetSurfaceFormat.format;
                }
                const VkPipelineRenderingCreateInfo rendering_info = {
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
                    .colorAttachmentCount = shader_info->color_attachment_count,
                    .pColorAttachmentFormats = color_formats,
                    .depthAttachmentFormat = Context::TargetDepthStencilFormat,
                    .stencilAttachmentFormat = Context::TargetDepthStencilFormat
                };

                /* Pipeline state */
                VkPipelineShaderStageCreateInfo shader_create_info[Context::TargetShaderStages - 3] = {};
                u32 stage_count = 0;
                if (vk_task_module != 0) {
                    shader_create_info[stage_count].sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                    shader_create_info[stage_count].stage  = VK_SHADER_STAGE_TASK_BIT_EXT;
                    shader_create_info[stage_count].module = vk_task_module;
                    shader_create_info[stage_count].pName   = "main";
                    ++stage_count;
                }
                if (vk_mesh_module != 0) {
                    shader_create_info[stage_count].sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                    shader_create_info[stage_count].stage  = VK_SHADER_STAGE_MESH_BIT_EXT;
                    shader_create_info[stage_count].module = vk_mesh_module;
                    shader_create_info[stage_count].pName   = "main";
                    ++stage_count;
                }
                if (vk_fragment_module != 0) {
                    shader_create_info[stage_count].sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                    shader_create_info[stage_count].stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
                    shader_create_info[stage_count].module = vk_fragment_module;
                    shader_create_info[stage_count].pName   = "main";
                    ++stage_count;
                }

                const VkPipelineViewportStateCreateInfo viewport_scissor_state = {
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO
                };

                const VkPipelineRasterizationStateCreateInfo rasterization_state = {
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO
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
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO
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
                    //VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY,
                    //VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE,
                    VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE,
                    VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE,
                    VK_DYNAMIC_STATE_DEPTH_COMPARE_OP,
                    VK_DYNAMIC_STATE_DEPTH_BOUNDS_TEST_ENABLE,
                    VK_DYNAMIC_STATE_STENCIL_TEST_ENABLE,
                    VK_DYNAMIC_STATE_STENCIL_OP,
                    VK_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE,
                    VK_DYNAMIC_STATE_DEPTH_BIAS_ENABLE,
                    //VK_DYNAMIC_STATE_PRIMITIVE_RESTART_ENABLE,

                    /* Ext vertex input */
                    //VK_DYNAMIC_STATE_VERTEX_INPUT_EXT,

                    /* Ext logic op */
                    VK_DYNAMIC_STATE_LOGIC_OP_EXT,

                    /* Ext 3 */
                    //VK_DYNAMIC_STATE_COLOR_BLEND_ADVANCED_EXT,
                    VK_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT,
                    VK_DYNAMIC_STATE_COLOR_BLEND_EQUATION_EXT,
                    VK_DYNAMIC_STATE_COLOR_WRITE_MASK_EXT,
                    //VK_DYNAMIC_STATE_CONSERVATIVE_RASTERIZATION_MODE_EXT,
                    VK_DYNAMIC_STATE_DEPTH_CLAMP_ENABLE_EXT,
                    VK_DYNAMIC_STATE_LOGIC_OP_ENABLE_EXT,
                    VK_DYNAMIC_STATE_POLYGON_MODE_EXT,
                    //VK_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT,
                    //VK_DYNAMIC_STATE_SAMPLE_MASK_EXT,
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
                m_pipeline_type = PipelineType_Mesh;
                
                /* Cleanup shader modules */
                if (vk_task_module != 0) {
                    ::pfn_vkDestroyShaderModule(context->GetDevice(), vk_task_module, nullptr);
                }
                if (vk_mesh_module != 0) {
                    ::pfn_vkDestroyShaderModule(context->GetDevice(), vk_mesh_module, nullptr);
                }
                if (vk_fragment_module != 0) {
                    ::pfn_vkDestroyShaderModule(context->GetDevice(), vk_fragment_module, nullptr);
                }
            }

            /* TODO; ray tracing pipeline */

            void Finalize(const Context *context) {
                if (m_vk_pipeline != 0) {
                    ::pfn_vkDestroyPipeline(context->GetDevice(), m_vk_pipeline, nullptr);
                    m_vk_pipeline = 0;
                }
                m_pipeline_type = 0;
            }

            constexpr ALWAYS_INLINE VkPipeline          GetPipeline()          const { return m_vk_pipeline; } 
            constexpr ALWAYS_INLINE VkPipelineBindPoint GetPipelineBindPoint() const {
                switch (m_pipeline_type) {
                    case PipelineType_Primitive:
                    case PipelineType_Mesh:
                        return VK_PIPELINE_BIND_POINT_GRAPHICS;
                    case PipelineType_RayTracing:
                        return VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR;
                    case PipelineType_Compute:
                        return VK_PIPELINE_BIND_POINT_COMPUTE;
                    default:
                        break;
                };
                return VK_PIPELINE_BIND_POINT_GRAPHICS;
            } 
    };
}
