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

namespace dd::learn {

    namespace {

        /* Vulkan objects */
        util::TypeStorage<vk::Shader>         vk_shader;
        util::TypeStorage<vk::DescriptorPool> vk_descriptor_pool;

        util::TypeStorage<vk::MemoryPool>     vk_memory;
        util::TypeStorage<vk::Buffer>         vk_vertex_buffer;
        util::TypeStorage<vk::Buffer>         vk_index_buffer;
        util::TypeStorage<vk::Buffer>         vk_uniform_buffer;
        unsigned char *texture0 = nullptr;
        unsigned char *texture1 = nullptr;
        util::TypeStorage<vk::Texture>        vk_texture0;
        util::TypeStorage<vk::Texture>        vk_texture1;
        util::TypeStorage<vk::TextureView>    vk_texture_view0;
        util::TypeStorage<vk::TextureView>    vk_texture_view1;
        vk::DescriptorSlot                    texture_view0_slot;
        vk::DescriptorSlot                    texture_view1_slot;

        /* Resources */
        const char *vertex_shader_path = "resources/2_vert.sh";
        const char *fragment_shader_path = "resources/2_frag.sh";

        const float vertices[] = {
            /* Position */      /* Color */        /* Tex Coords*/
            0.5f,  0.5f, 0.0f,  0.1f, 0.2f, 0.5f,  1.0f, 1.0f,
            0.5f, -0.5f, 0.0f,  0.7f, 0.2f, 0.5f,  1.0f, 0.0f,
           -0.5f, -0.5f, 0.0f,  0.1f, 0.6f, 0.9f,  0.0f, 0.0f,
           -0.5f,  0.5f, 0.0f,  0.2f, 0.2f, 0.5f,  0.0f, 1.0f
        };
        constexpr inline u32 VerticeCount = 4;
        constexpr inline u32 VerticeStride = sizeof(vertices) / VerticeCount;
        const u32 indices[] = {
            0, 1, 3,
            1, 2, 3
        };

        const dd::util::math::Matrix34f base_transform(0.5f, 0.0f, 0.0f, 0.5f, 
                                                       0.0f, 1.5f, 0.0f, -0.3f,
                                                       0.0f, 0.0f, 1.0f, 0.0f);
        dd::util::math::Matrix34f rot_mtx = dd::util::math::IdentityMatrix34<float>;
        float rot_z_angle = 0.0f;

        char *memory_buffer = nullptr;
        
        /* Vulkan pipeline state */
        util::TypeStorage<vk::Pipeline>       vk_pipeline;
        vk::PipelineCmdState                  vk_pipeline_cmd_state;

        const VkVertexInputBindingDescription2EXT vk_input_binding_description = {
            .sType     = VK_STRUCTURE_TYPE_VERTEX_INPUT_BINDING_DESCRIPTION_2_EXT,
            .binding   = 0,
            .stride    = VerticeStride,
            .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
            .divisor   = 1
        };
        constexpr size_t input_binding_count = sizeof(vk_input_binding_description) / sizeof(VkVertexInputBindingDescription2EXT);

        const VkVertexInputAttributeDescription2EXT vk_attribute_descriptions[] = {
            {
                .sType = VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT,
                .location = 0,
                .binding = 0,
                .format = VK_FORMAT_R32G32B32_SFLOAT,
                .offset = 0
            },
            {
                .sType = VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT,
                .location = 1,
                .binding = 0,
                .format = VK_FORMAT_R32G32B32_SFLOAT,
                .offset = sizeof(float) * 3
            },
            {
                .sType = VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT,
                .location = 2,
                .binding = 0,
                .format = VK_FORMAT_R32G32_SFLOAT,
                .offset = sizeof(float) * 6
            },
        };
        constexpr size_t input_attribute_count = sizeof(vk_attribute_descriptions) / sizeof(VkVertexInputAttributeDescription2EXT);
    }

    void SetupTriangle() {

        vk::Context *context = vk::GetGlobalContext();

        /* Load Shader */
        char *vertex_shader = nullptr;
        char *fragment_shader = nullptr;
        res::LoadTextFile(vertex_shader_path, std::addressof(vertex_shader), nullptr);
        res::LoadTextFile(fragment_shader_path, std::addressof(fragment_shader), nullptr);

        /* Create Shader */
        dd::util::ConstructAt(vk_shader);
        const vk::ShaderInfo shader_info = {
            .vertex_code_size = ::strlen(vertex_shader) + 1,
            .vertex_code = reinterpret_cast<u32*>(vertex_shader),
            .fragment_code_size = ::strlen(fragment_shader) + 1,
            .fragment_code = reinterpret_cast<u32*>(fragment_shader)
        };
        util::GetReference(vk_shader).Initialize(context, std::addressof(shader_info));

        /* Load textures */
        s32 width0 = 0, height0 = 0, channels0 = 0;
        s32 width1 = 0, height1 = 0, channels1 = 0;
        res::LoadStbImage("resources/woodcrate.jpg", 4, std::addressof(texture0), std::addressof(width0), std::addressof(height0), std::addressof(channels0));
        res::LoadStbImage("resources/awesomeface.png", 0, std::addressof(texture1), std::addressof(width1), std::addressof(height1), std::addressof(channels1));

        const s64 texture0_size = width0 * height0 * channels0;
        const s64 texture1_size = width1 * height1 * channels1;

        /* Create resource infos */
        vk::BufferInfo vertex_buffer_info = {
            .size = sizeof(vertices),
            .vk_usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT
        };

        vk::BufferInfo index_buffer_info = {
            .size = sizeof(vertices),
            .vk_usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT
        };

        vk::BufferInfo uniform_buffer_info = {
            .size = sizeof(vertices),
            .vk_usage = VK_IMAGE_USAGE_SAMPLED_BIT
        };
        
        vk::TextureInfo texture0_info = {
            .vk_usage_flags       = VK_IMAGE_USAGE_SAMPLED_BIT,
            .mip_levels           = 1,
            .width                = static_cast<u32>(width0),
            .height               = static_cast<u32>(height0),
            .depth                = 1,
            .vk_format            = VK_FORMAT_R8G8B8A8_UNORM,
            .vk_image_layout      = VK_IMAGE_LAYOUT_PREINITIALIZED,
            .vk_image_type        = VK_IMAGE_TYPE_2D,
            .vk_sample_count_flag = VK_SAMPLE_COUNT_1_BIT,
            .vk_tiling            = VK_IMAGE_TILING_OPTIMAL,
            .array_layers         = 1
        };
        
        vk::TextureInfo texture1_info = {
            .vk_usage_flags       = VK_IMAGE_USAGE_SAMPLED_BIT,
            .mip_levels           = 1,
            .width                = static_cast<u32>(width1),
            .height               = static_cast<u32>(height1),
            .depth                = 1,
            .vk_format            = VK_FORMAT_R8G8B8A8_UNORM,
            .vk_image_layout      = VK_IMAGE_LAYOUT_PREINITIALIZED,
            .vk_image_type        = VK_IMAGE_TYPE_2D,
            .vk_sample_count_flag = VK_SAMPLE_COUNT_1_BIT,
            .vk_tiling            = VK_IMAGE_TILING_OPTIMAL,
            .array_layers         = 1
        };
        
        /* Calculate offsets */
        index_buffer_info.offset    = util::AlignUp(sizeof(vertices) , vk::Buffer::GetAlignment(context, std::addressof(index_buffer_info)));
        uniform_buffer_info.offset  = util::AlignUp(sizeof(vertices) + sizeof(indices), vk::Buffer::GetAlignment(context, std::addressof(uniform_buffer_info)));

        texture0_info.memory_offset = util::AlignUp(sizeof(vertices) + sizeof(indices) + sizeof(base_transform), vk::Texture::GetAlignment(context, std::addressof(texture0_info)));
        texture1_info.memory_offset = util::AlignUp(sizeof(vertices) + sizeof(indices) + sizeof(base_transform) + texture0_size, vk::Texture::GetAlignment(context, std::addressof(texture1_info)));

        /* Determine memory size */
        const u64 buffer_size = util::AlignUp(texture1_info.memory_offset + texture1_size, vk::Context::TargetMemoryPoolAlignment);

        /* Create GPU host buffer */
        memory_buffer = new(std::align_val_t(vk::Context::TargetMemoryPoolAlignment)) char[buffer_size];
        DD_ASSERT(memory_buffer != nullptr);
        ::memcpy(memory_buffer, std::addressof(vertices), sizeof(vertices));
        ::memcpy(reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(memory_buffer) + index_buffer_info.offset), std::addressof(indices), sizeof(indices));
        ::memcpy(reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(memory_buffer) + uniform_buffer_info.offset), std::addressof(base_transform), sizeof(base_transform));
        ::memcpy(reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(memory_buffer) + texture0_info.memory_offset), texture0, texture0_size);
        ::memcpy(reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(memory_buffer) + texture1_info.memory_offset), texture1, texture1_size);

        /* Free unneeded image buffers */
        res::FreeStbImage(texture0);
        res::FreeStbImage(texture1);

        /* Create memory pool */
        util::ConstructAt(vk_memory);
        const vk::MemoryPoolInfo pool_info = {
            .size = buffer_size,
            .vk_memory_property_flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            .import_memory = memory_buffer
        };
        vk::MemoryPool *pool = util::GetPointer(vk_memory);
        pool->Initialize(context, std::addressof(pool_info));

        /* Create buffers */
        util::ConstructAt(vk_vertex_buffer);
        util::GetReference(vk_vertex_buffer).Initialize(context, std::addressof(vertex_buffer_info), pool);

        util::ConstructAt(vk_index_buffer);
        util::GetReference(vk_index_buffer).Initialize(context, std::addressof(index_buffer_info), pool);

        util::ConstructAt(vk_uniform_buffer);
        util::GetReference(vk_uniform_buffer).Initialize(context, std::addressof(uniform_buffer_info), pool);

        /* Create textures */
        util::ConstructAt(vk_texture0);
        util::GetReference(vk_texture0).Initialize(context, std::addressof(texture0_info), pool);

        util::ConstructAt(vk_texture1);
        util::GetReference(vk_texture1).Initialize(context, std::addressof(texture1_info), pool);

        /* Create texture views */
        vk::TextureViewInfo view_info = {};
        view_info.SetDefaults();

        view_info.vk_format  = VK_FORMAT_R8G8B8_UNORM;
        view_info.texture = util::GetPointer(vk_texture0);

        util::ConstructAt(vk_texture_view0);
        util::GetReference(vk_texture_view0).Initialize(context, std::addressof(view_info));

        view_info.vk_format  = VK_FORMAT_R8G8B8A8_UNORM;
        view_info.texture = util::GetPointer(vk_texture1);

        util::ConstructAt(vk_texture_view1);
        util::GetReference(vk_texture_view1).Initialize(context, std::addressof(view_info));

        /* Create pipeline */
        vk::PipelineInfo pipeline_info = {};
        pipeline_info.SetDefaults();
        pipeline_info.shader = util::GetPointer(vk_shader);

        util::ConstructAt(vk_pipeline);
        util::GetReference(vk_pipeline).Initialize(context, std::addressof(pipeline_info));

        /* Set our cmd state */
        vk_pipeline_cmd_state.SetDefaults();
        vk_pipeline_cmd_state.vertex_state.vertex_binding_count = input_binding_count;
        vk_pipeline_cmd_state.vertex_state.vertex_attribute_count = input_attribute_count;
        vk_pipeline_cmd_state.vertex_state.vertex_binding_array = std::addressof(vk_input_binding_description);
        vk_pipeline_cmd_state.vertex_state.vertex_attribute_array = vk_attribute_descriptions;

        /* Create descriptor pool */
        vk::DescriptorPool *descriptor_pool = util::GetPointer(vk_descriptor_pool);

        util::ConstructAt(vk_descriptor_pool);
        descriptor_pool->Initialize(context, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 32);

        /* Register our textures */
        texture_view0_slot = descriptor_pool->RegisterTexture(util::GetPointer(vk_texture_view0));
        texture_view1_slot = descriptor_pool->RegisterTexture(util::GetPointer(vk_texture_view1));
    }
    
    void CalcTriangle() {

        /* Calculate new matrix */
        rot_mtx = base_transform;
        rot_z_angle += dd::util::math::TRadians<float, 1.0f> * dd::util::GetDeltaTime();
        ::fmod(rot_z_angle, dd::util::math::Float2Pi);
        dd::util::math::RotateLocalZ(std::addressof(rot_mtx), rot_z_angle);
    }
    
    void DrawTriangle(vk::CommandBuffer *command_buffer) {

        /* Copy matrix to buffer */
        void *ubo_address = util::GetReference(vk_uniform_buffer).Map();
        ::memcpy(ubo_address, std::addressof(rot_mtx), sizeof(rot_mtx));
        util::GetReference(vk_uniform_buffer).Unmap();

        /* Bind */
        command_buffer->SetPipeline(util::GetPointer(vk_pipeline));
        command_buffer->SetPipelineState(std::addressof(vk_pipeline_cmd_state));
        
        command_buffer->SetVertexBuffer(0, util::GetPointer(vk_vertex_buffer), VerticeStride, sizeof(vertices));
        
        command_buffer->SetUniformBuffer(0, vk::ShaderStage_Vertex, util::GetReference(vk_uniform_buffer).GetGpuAddress());

        VkViewport viewport {
            .width = 1.0f,
            .height = 1.0f
        };
        command_buffer->SetViewports(1, std::addressof(viewport));

        /* Draw */
        command_buffer->DrawIndexed(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_INDEX_TYPE_UINT32, util::GetPointer(vk_index_buffer), 0, 6);
    }

    void CleanTriangle() {
        vk::Context *context = vk::GetGlobalContext();

        util::GetReference(vk_shader).Finalize(context);
        dd::util::DestructAt(vk_shader);
    }
}
