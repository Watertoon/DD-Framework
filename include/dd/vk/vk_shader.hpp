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

    enum ShaderStage {
        ShaderStage_Vertex                 = 0,
        ShaderStage_TessellationEvaluation = 1,
        ShaderStage_TessellationControl    = 2,
        ShaderStage_Geometry               = 3,
        ShaderStage_Fragment               = 4,
        ShaderStage_Compute                = 5,
    };

    struct ShaderInfo {
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
        size_t  compute_code_size;
        u32    *compute_code;
    };

    class Shader {
        private:
            VkShaderModule m_vk_vertex_module;
            VkShaderModule m_vk_tessellation_control_module;
            VkShaderModule m_vk_tessellation_evaluation_module;
            VkShaderModule m_vk_geometry_module;
            VkShaderModule m_vk_fragment_module;
            VkShaderModule m_vk_compute_module;
        public:
            constexpr Shader() {/*...*/}

            void Initialize(const Context *context, const ShaderInfo *shader_info) {
                if (shader_info->vertex_code != nullptr) {
                    const VkShaderModuleCreateInfo shader_module_info = {
                        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
                        .codeSize = shader_info->vertex_code_size,
                        .pCode = shader_info->vertex_code
                    };
                    const u32 result = ::pfn_vkCreateShaderModule(context->GetDevice(), std::addressof(shader_module_info), nullptr, std::addressof(m_vk_vertex_module));
                    DD_ASSERT(result == VK_SUCCESS);
                }
                if (shader_info->tessellation_control_code != nullptr) {
                    const VkShaderModuleCreateInfo shader_module_info = {
                        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
                        .codeSize = shader_info->tessellation_control_code_size,
                        .pCode = shader_info->tessellation_control_code
                    };
                    const u32 result = ::pfn_vkCreateShaderModule(context->GetDevice(), std::addressof(shader_module_info), nullptr, std::addressof(m_vk_tessellation_control_module));
                    DD_ASSERT(result == VK_SUCCESS);
                }
                if (shader_info->tessellation_evaluation_code != nullptr) {
                    const VkShaderModuleCreateInfo shader_module_info = {
                        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
                        .codeSize = shader_info->tessellation_evaluation_code_size,
                        .pCode = shader_info->tessellation_evaluation_code
                    };
                    const u32 result = ::pfn_vkCreateShaderModule(context->GetDevice(), std::addressof(shader_module_info), nullptr, std::addressof(m_vk_tessellation_evaluation_module));
                    DD_ASSERT(result == VK_SUCCESS);
                }
                if (shader_info->geometry_code != nullptr) {
                    const VkShaderModuleCreateInfo shader_module_info = {
                        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
                        .codeSize = shader_info->geometry_code_size,
                        .pCode = shader_info->geometry_code
                    };
                    const u32 result = ::pfn_vkCreateShaderModule(context->GetDevice(), std::addressof(shader_module_info), nullptr, std::addressof(m_vk_geometry_module));
                    DD_ASSERT(result == VK_SUCCESS);
                }
                if (shader_info->fragment_code != nullptr) {
                    const VkShaderModuleCreateInfo shader_module_info = {
                        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
                        .codeSize = shader_info->fragment_code_size,
                        .pCode = shader_info->fragment_code
                    };
                    const u32 result = ::pfn_vkCreateShaderModule(context->GetDevice(), std::addressof(shader_module_info), nullptr, std::addressof(m_vk_fragment_module));
                    DD_ASSERT(result == VK_SUCCESS);
                }
                if (shader_info->compute_code != nullptr) {
                    const VkShaderModuleCreateInfo shader_module_info = {
                        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
                        .codeSize = shader_info->compute_code_size,
                        .pCode = shader_info->compute_code
                    };
                    const u32 result = ::pfn_vkCreateShaderModule(context->GetDevice(), std::addressof(shader_module_info), nullptr, std::addressof(m_vk_compute_module));
                    DD_ASSERT(result == VK_SUCCESS);
                }
            }

            void Finalize(const Context *context) {
                if (m_vk_vertex_module != 0) {
                    ::pfn_vkDestroyShaderModule(context->GetDevice(), m_vk_vertex_module, nullptr);
                }
                if (m_vk_tessellation_control_module != 0) {
                    ::pfn_vkDestroyShaderModule(context->GetDevice(), m_vk_tessellation_control_module, nullptr);
                }
                if (m_vk_tessellation_evaluation_module != 0) {
                    ::pfn_vkDestroyShaderModule(context->GetDevice(), m_vk_tessellation_evaluation_module, nullptr);
                }
                if (m_vk_geometry_module != 0) {
                    ::pfn_vkDestroyShaderModule(context->GetDevice(), m_vk_geometry_module, nullptr);
                }
                if (m_vk_fragment_module != 0) {
                    ::pfn_vkDestroyShaderModule(context->GetDevice(), m_vk_fragment_module, nullptr);
                }
                if (m_vk_compute_module != 0) {
                    ::pfn_vkDestroyShaderModule(context->GetDevice(), m_vk_compute_module, nullptr);
                }
            }

            constexpr ALWAYS_INLINE VkShaderModule GetVertexModule() const { return m_vk_vertex_module; }

            constexpr ALWAYS_INLINE VkShaderModule GetTessellationControlModule() const { return m_vk_tessellation_control_module; }

            constexpr ALWAYS_INLINE VkShaderModule GetTessellationEvaluationModule() const { return m_vk_tessellation_evaluation_module; }

            constexpr ALWAYS_INLINE VkShaderModule GetGeometryModule() const { return m_vk_geometry_module; }

            constexpr ALWAYS_INLINE VkShaderModule GetFragmentModule() const { return m_vk_fragment_module; }

            constexpr ALWAYS_INLINE VkShaderModule GetComputeModule() const { return m_vk_compute_module; }
    };
}
