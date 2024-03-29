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

    struct ColorBlendCmdState {
        u32                     color_blend_count;
        VkBool32                color_blend_enables[Context::TargetColorAttachmentCount];
        VkColorBlendEquationEXT vk_color_blend_equations[Context::TargetColorAttachmentCount];
        VkColorComponentFlags   vk_color_blend_write_masks[Context::TargetColorAttachmentCount];
        bool                    logic_op_enable;
        VkLogicOp               vk_logic_op;
        float                   blend_constants[4];

        constexpr void SetDefaults() {
            vk_logic_op        = VK_LOGIC_OP_COPY,
            blend_constants[0] = 0.0f;
            blend_constants[1] = 0.0f;
            blend_constants[2] = 0.0f;
            blend_constants[3] = 1.0f;
            for (u32 i = 0; i < Context::TargetColorAttachmentCount; ++i) {
                color_blend_enables[i]        = false;
                vk_color_blend_write_masks[i] = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
            }
            color_blend_count = 1;
        }
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

        constexpr void SetDefaults() {
            depth_test_enable             = true;
            depth_write_enable            = true;
            depth_bounds_test_enable      = false;
            stencil_test_enable           = false;
            vk_op_state_front.failOp      = VK_STENCIL_OP_KEEP;
            vk_op_state_front.passOp      = VK_STENCIL_OP_KEEP;
            vk_op_state_front.depthFailOp = VK_STENCIL_OP_KEEP;
            vk_op_state_front.compareOp   = VK_COMPARE_OP_ALWAYS;
            vk_op_state_front.compareMask = 0;
            vk_op_state_front.writeMask   = 0;
            vk_op_state_front.reference   = 0;
            vk_op_state_back.failOp       = VK_STENCIL_OP_KEEP;
            vk_op_state_back.passOp       = VK_STENCIL_OP_KEEP;
            vk_op_state_back.depthFailOp  = VK_STENCIL_OP_KEEP;
            vk_op_state_back.compareOp    = VK_COMPARE_OP_ALWAYS;
            vk_op_state_back.compareMask  = 0;
            vk_op_state_back.writeMask    = 0;
            vk_op_state_back.reference    = 0;
            vk_depth_compare_op           = VK_COMPARE_OP_GREATER;
            min_depth_bounds              = 0.0f;
            max_depth_bounds              = 1.0f;
        }
    };

    struct RasterizerCmdState {
        VkCullModeFlags vk_cull_mode;
        VkFrontFace     vk_front_face;
        VkPolygonMode   vk_polygon_mode;
        bool            rasterizer_discard_enable;
        bool            primitive_restart_enable;
        bool            depth_bias_enable;
        bool            depth_clamp_enable;
        float           depth_bias_constant_factor;
        float           depth_bias_clamp;
        float           depth_bias_slope_factor;
        float           line_width;

        constexpr void SetDefaults() {
            vk_cull_mode               = VK_CULL_MODE_NONE;
            vk_front_face              = VK_FRONT_FACE_CLOCKWISE;
            rasterizer_discard_enable  = false;
            primitive_restart_enable   = false;
            depth_bias_enable          = false;
            depth_clamp_enable         = false;
            vk_polygon_mode            = VK_POLYGON_MODE_FILL;
            depth_bias_constant_factor = 0.0f;
            depth_bias_clamp           = 0.0f;
            depth_bias_slope_factor    = 0.0f;
            line_width                 = 1.0f;
        }
    };

    struct VertexCmdState {
        u32                                          vertex_binding_count;
        u32                                          vertex_attribute_count;
        const VkVertexInputBindingDescription2EXT   *vertex_binding_array;
        const VkVertexInputAttributeDescription2EXT *vertex_attribute_array;

        constexpr void SetDefaults() {
            vertex_binding_count   = 0;
            vertex_attribute_count = 0;
            vertex_binding_array   = nullptr;
            vertex_attribute_array = nullptr;
        }
    };

    struct PipelineCmdState {
        ColorBlendCmdState   color_blend_state;
        DepthStencilCmdState depth_stencil_state;
        RasterizerCmdState   rasterizer_state;
        VertexCmdState       vertex_state;
        
        constexpr void SetDefaults() {
            color_blend_state.SetDefaults();
            depth_stencil_state.SetDefaults();
            rasterizer_state.SetDefaults();
            color_blend_state.SetDefaults();
        }
    };

    struct BufferBarrierCmdState {
        VkPipelineStageFlags2 vk_src_stage_mask;
        VkPipelineStageFlags2 vk_dst_stage_mask;
        VkAccessFlags2        vk_src_access_mask;
        VkAccessFlags2        vk_dst_access_mask;

        void SetDefaults() {
            vk_src_stage_mask  = VK_ACCESS_NONE;
            vk_dst_stage_mask  = VK_ACCESS_NONE;
            vk_src_access_mask = VK_ACCESS_NONE;
            vk_dst_access_mask = VK_ACCESS_NONE;
        }
    };

    struct TextureBarrierCmdState {
        VkPipelineStageFlags2 vk_src_stage_mask;
        VkPipelineStageFlags2 vk_dst_stage_mask;
        VkAccessFlags2        vk_src_access_mask;
        VkAccessFlags2        vk_dst_access_mask;
        VkImageLayout         vk_src_layout;
        VkImageLayout         vk_dst_layout;

        void SetDefaults() {
            vk_src_stage_mask  = VK_ACCESS_NONE;
            vk_dst_stage_mask  = VK_ACCESS_NONE;
            vk_src_access_mask = VK_ACCESS_NONE;
            vk_dst_access_mask = VK_ACCESS_NONE;
            vk_src_layout      = VK_IMAGE_LAYOUT_UNDEFINED;
            vk_dst_layout      = VK_IMAGE_LAYOUT_UNDEFINED;
        }
    };

    class CommandBuffer {
        private:
            friend class Context;
        private:
            struct ResourceBuffer {
                VkDeviceAddress uniform_buffers[Context::TargetMaxPerStageUniformBufferCount];
                VkDeviceAddress storage_buffers[Context::TargetMaxPerStageStorageBufferCount];
                u32             texture_ids    [Context::TargetMaxPerStageTextureCount];
                u32             sampler_ids    [Context::TargetMaxPerStageTextureCount];
                u64             reserve0;
                u64             reserve1;

                constexpr void SetDefaults() {
                    ::memset(this, 0, sizeof(ResourceBuffer));
                }
            };
            static_assert(sizeof(ResourceBuffer) == 0x200);
        public:
            static constexpr size_t TargetMaxResourceUpdatesPerSubmission    = 120;
            static constexpr size_t TargetResourceBufferPerStageSize         = util::AlignUp(sizeof(ResourceBuffer) * TargetMaxResourceUpdatesPerSubmission, Context::TargetConstantBufferAlignment);
            static_assert(TargetResourceBufferPerStageSize <= Context::TargetMaxUniformBufferSize);
            static constexpr size_t TargetResourceBufferMemorySize           = util::AlignUp(TargetResourceBufferPerStageSize * 6, Context::TargetMemoryPoolAlignment);

            static constexpr size_t VertexBufferMemoryOffset                 = 0;
            static constexpr size_t TessellationControlBufferMemoryOffset    = TargetResourceBufferPerStageSize;
            static constexpr size_t TessellationEvaluationBufferMemoryOffset = TargetResourceBufferPerStageSize * 2;
            static constexpr size_t GeometryBufferMemoryOffset               = TargetResourceBufferPerStageSize * 3;
            static constexpr size_t FragmentBufferMemoryOffset               = TargetResourceBufferPerStageSize * 4;
            static constexpr size_t ComputeBufferMemoryOffset                = TargetResourceBufferPerStageSize * 5;
        private:
            VkCommandBuffer           m_vk_command_buffer;
            ResourceBuffer            m_resource_buffer_per_stage_array[Context::TargetShaderStages];
            void                     *m_resource_buffer_mapped_address;
            VkDeviceMemory            m_vk_resource_buffer_memory;
            VkBuffer                  m_vk_resource_buffer;
            VkDeviceAddress           m_vk_resource_buffer_address;
            ColorTargetView          *m_color_targets[8];
            DepthStencilTargetView   *m_depth_stencil_target;
            VkClearColorValue         m_vk_clear_color;
            VkClearDepthStencilValue  m_vk_clear_depth_stencil;
            bool                      m_is_rendering;
            bool                      m_are_new_targets;
            bool                      m_need_vertex_resource_update;
            bool                      m_need_tessellation_evaluation_resource_update;
            bool                      m_need_tessellation_control_resource_update;
            bool                      m_need_geometry_resource_update;
            bool                      m_need_fragment_resource_update;
            bool                      m_need_compute_resource_update;
            u32                       m_resource_update_count;
            u16                       m_render_width;
            u16                       m_render_height;
            bool                      m_fast_clear_color;
            bool                      m_fast_clear_depth_stencil;
        private:
            void BeginRenderingIfNotRendering();

            void EndRenderingIfRendering();

            void UpdateResourceBufferIfNecessary();
        public:
            constexpr CommandBuffer() { /*...*/ }

            void Initialize(const Context *context, size_t draw_memory_size);
            void Finalize(const Context *context);

            void Begin();
            void End();

            void ClearColorTarget(ColorTargetView *color_target, const VkClearColorValue* color, const VkImageSubresourceRange *sub_range);
            void ClearDepthStencilTarget(DepthStencilTargetView *depth_stencil_target, const VkClearDepthStencilValue clear_value, const VkImageSubresourceRange *sub_range);
            void RegisterFastClearColor(const VkClearColorValue& color_value);
            void RegisterFastClearDepthStencil(const VkClearDepthStencilValue& depth_stencil_value);

            void Draw(VkPrimitiveTopology vk_primitive_topology, u32 vertex_count, u32 base_vertex);
            void DrawIndexed(VkPrimitiveTopology vk_primitive_topology, VkIndexType index_format, Buffer *index_buffer, u32 index_count, u32 base_index);

            void DrawInstanced(VkPrimitiveTopology vk_primitive_topology, u32 vertex_count, u32 base_vertex, u32 instance_count, u32 base_instance);
            void DrawInstancedIndexed(VkPrimitiveTopology vk_primitive_topology, VkIndexType index_format, Buffer *index_buffer, u32 index_count, u32 base_index, u32 instance_count, u32 base_instance);

            void SetShader(Shader *shader);
            void SetPipelineState(const PipelineCmdState *pipeline_state);
            void SetColorBlendState(const ColorBlendCmdState *color_blend_state);
            void SetDepthStencilState(const DepthStencilCmdState *depth_stencil_state);
            void SetRasterizerState(const RasterizerCmdState *rasterizer_state);
            void SetVertexState(const VertexCmdState *vertex_state);

            void SetViewports(u32 viewport_count, const VkViewport *viewport_array);
            void SetScissors(u32 scissor_count, const VkRect2D *scissor_array);
            void SetViewportScissors(u32 viewport_count, const VkViewport *viewport_array, u32 scissor_count, const VkRect2D *scissor_array);

            void SetRenderTargets(s32 color_target_count, ColorTargetView **color_targets, DepthStencilTargetView *depth_stencil_target);

            void SetVertexBuffer(u32 binding, Buffer *vertex_buffer, size_t stride, size_t size);

            void SetBufferStateTransition(Buffer *buffer, const BufferBarrierCmdState *barrier_state);
            void SetTextureStateTransition(Texture *texture, const TextureBarrierCmdState *barrier_state, VkImageAspectFlagBits aspect_mask);

            void SetDescriptorPool(const DescriptorPool *descriptor_pool);

            void SetUniformBuffer(u32 location, ShaderStage shader_stage, const VkDeviceAddress gpu_address);
            void SetStorageBuffer(u32 location, ShaderStage shader_stage, const VkDeviceAddress gpu_address);
            void SetTextureAndSampler(u32 location, ShaderStage shader_stage, const DescriptorSlot texture_slot, const DescriptorSlot sampler_slot);

            void SetRenderDimensions(u32 width, u32 height) {
                m_render_width  = width;
                m_render_height = height;
            }
            constexpr VkCommandBuffer GetCommandBuffer() const { return m_vk_command_buffer; }
            static constexpr size_t GetDrawMemorySize(u32 max_shader_stages, u32 max_draws) { return max_shader_stages * max_draws * TargetResourceBufferPerStageSize; }
    };
}
