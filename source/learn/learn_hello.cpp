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
        util::TypeStorage<vk::DescriptorPool> vk_texture_descriptor_pool;
        util::TypeStorage<vk::DescriptorPool> vk_sampler_descriptor_pool;

        util::TypeStorage<vk::MemoryPool>     vk_buffer_memory;
        util::TypeStorage<vk::Buffer>         vk_vertex_buffer;
        util::TypeStorage<vk::Buffer>         vk_index_buffer;
        util::TypeStorage<vk::Buffer>         vk_uniform_buffer;
        unsigned char *texture0 = nullptr;
        unsigned char *texture1 = nullptr;
        util::TypeStorage<vk::MemoryPool>     vk_image_memory;
        util::TypeStorage<vk::Texture>        vk_texture0;
        util::TypeStorage<vk::Texture>        vk_texture1;
        util::TypeStorage<vk::Sampler>        vk_sampler;
        util::TypeStorage<vk::TextureView>    vk_texture_view0;
        util::TypeStorage<vk::TextureView>    vk_texture_view1;
        vk::DescriptorSlot                    texture_view0_slot;
        vk::DescriptorSlot                    texture_view1_slot;
        vk::DescriptorSlot                    sampler_slot;

        /* Resources */
        const char *vertex_shader_path = "shaders/primitive_vertex.spv";
        const char *fragment_shader_path = "shaders/primitive_fragment.spv";

        const float vertices[] = {
            /* Position */       /* Tex Coords */
            -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
             0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
             0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
             0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
            -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
             0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
             0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
             0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
            -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

            -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
            -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
            -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

             0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
             0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
             0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
             0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
             0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
             0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

            -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
             0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
             0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
             0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

            -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
             0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
             0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
             0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
            -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
            -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
        };
        constexpr inline u32 VerticeCount = sizeof(vertices) / (sizeof(float) * 5);
        static_assert(VerticeCount == 36);
        constexpr inline u32 VerticeStride = sizeof(vertices) / VerticeCount;
        const u32 indices[] = {
            0, 1, 3,
            1, 2, 3
        };

        const util::math::Vector3f CubePositions[] = {
            util::math::Vector3f( 0.0f,  0.0f,  0.0f), 
            util::math::Vector3f( 2.0f,  5.0f, -15.0f), 
            util::math::Vector3f(-1.5f, -2.2f, -2.5f),  
            util::math::Vector3f(-3.8f, -2.0f, -12.3f),  
            util::math::Vector3f( 2.4f, -0.4f, -3.5f),  
            util::math::Vector3f(-1.7f,  3.0f, -7.5f),  
            util::math::Vector3f( 1.3f, -2.0f, -2.5f),  
            util::math::Vector3f( 1.5f,  2.0f, -2.5f), 
            util::math::Vector3f( 1.5f,  0.2f, -1.5f), 
            util::math::Vector3f(-1.3f,  1.0f, -1.5f)  
        };
        constexpr inline u32 CubeCount = sizeof(CubePositions) / sizeof(util::math::Vector3f);

        dd::util::math::Matrix34f model_matrix = util::math::IdentityMatrix34<float>;
        dd::util::LookAtCamera camera = {{ 0.0f, 0.0f, 3.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }};
        dd::util::PerspectiveProjection perspective_projection(0.1f, 100.0f, util::math::TRadians<float, 45.0f>, 1280.0f / 720.0f);

        struct ViewArg {
            dd::util::math::Matrix34f model_matrix;
            dd::util::math::Matrix34f view_matrix;
            dd::util::math::Matrix44f projection_matrix;
        };

        constexpr u32 UniformBufferSize = sizeof(ViewArg) * CubeCount;

        float yaw = -90.0f;
        float pitch = 0.0f;

        char *memory_buffer = nullptr;
        char *memory_image = nullptr;

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
                .format = VK_FORMAT_R32G32_SFLOAT,
                .offset = sizeof(float) * 3
            },
        };
        constexpr size_t input_attribute_count = sizeof(vk_attribute_descriptions) / sizeof(VkVertexInputAttributeDescription2EXT);
    }

    void SetupTriangle() {

        vk::Context *context = vk::GetGlobalContext();

        /* Load Shader */
        char *vertex_shader = nullptr;
        char *fragment_shader = nullptr;
        u32 vertex_size = 0, fragment_size = 0;
        res::LoadFile(vertex_shader_path, reinterpret_cast<void**>(std::addressof(vertex_shader)), std::addressof(vertex_size));
        res::LoadFile(fragment_shader_path, reinterpret_cast<void**>(std::addressof(fragment_shader)), std::addressof(fragment_size));

        /* Create Shader */
        dd::util::ConstructAt(vk_shader);
        const vk::ShaderInfo shader_info = {
            .vertex_code_size = vertex_size,
            .vertex_code = reinterpret_cast<u32*>(vertex_shader),
            .fragment_code_size = fragment_size,
            .fragment_code = reinterpret_cast<u32*>(fragment_shader)
        };
        util::GetReference(vk_shader).Initialize(context, std::addressof(shader_info));

        /* Load textures */
        s32 width0 = 0, height0 = 0, channels0 = 0;
        s32 width1 = 0, height1 = 0, channels1 = 0;
        res::LoadStbImage("resources/third_party/woodcrate.jpg", 4, std::addressof(texture0), std::addressof(width0), std::addressof(height0), std::addressof(channels0));
        res::LoadStbImage("resources/third_party/awesomeface.png", 0, std::addressof(texture1), std::addressof(width1), std::addressof(height1), std::addressof(channels1));

        const s64 texture0_size = width0 * height0 * 4;
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
            .vk_usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT
        };
        
        vk::TextureInfo texture0_info = {
            .vk_usage_flags       = VK_IMAGE_USAGE_SAMPLED_BIT,
            .mip_levels           = 1,
            .width                = static_cast<u32>(width0),
            .height               = static_cast<u32>(height0),
            .depth                = 1,
            .vk_format            = VK_FORMAT_R8G8B8A8_UNORM,
            .vk_image_layout      = VK_IMAGE_LAYOUT_UNDEFINED,
            .vk_image_type        = VK_IMAGE_TYPE_2D,
            .vk_sample_count_flag = VK_SAMPLE_COUNT_1_BIT,
            .vk_tiling            = VK_IMAGE_TILING_LINEAR,
            .array_layers         = 1,
            .memory_offset        = 0
        };
        
        vk::TextureInfo texture1_info = {
            .vk_usage_flags       = VK_IMAGE_USAGE_SAMPLED_BIT,
            .mip_levels           = 1,
            .width                = static_cast<u32>(width1),
            .height               = static_cast<u32>(height1),
            .depth                = 1,
            .vk_format            = VK_FORMAT_R8G8B8A8_UNORM,
            .vk_image_layout      = VK_IMAGE_LAYOUT_UNDEFINED,
            .vk_image_type        = VK_IMAGE_TYPE_2D,
            .vk_sample_count_flag = VK_SAMPLE_COUNT_1_BIT,
            .vk_tiling            = VK_IMAGE_TILING_LINEAR,
            .array_layers         = 1
        };
        
        /* Calculate offsets */
        index_buffer_info.offset    = util::AlignUp(sizeof(vertices), vk::Buffer::GetAlignment(context, std::addressof(index_buffer_info)));
        uniform_buffer_info.offset  = util::AlignUp(index_buffer_info.offset + sizeof(indices), vk::Buffer::GetAlignment(context, std::addressof(uniform_buffer_info)));

        texture1_info.memory_offset = util::AlignUp(texture0_size, vk::Texture::GetAlignment(context, std::addressof(texture1_info)));

        /* Determine memory size */
        const u64 buffer_memory_size = util::AlignUp(uniform_buffer_info.offset + UniformBufferSize, vk::Context::TargetMemoryPoolAlignment);
        const u64 image_memory_size = util::AlignUp(texture1_info.memory_offset + texture1_size, vk::Context::TargetMemoryPoolAlignment);

        /* Copy host memory */
        memory_buffer = new(std::align_val_t(vk::Context::TargetMemoryPoolAlignment)) char[buffer_memory_size];
        memory_image  = new(std::align_val_t(vk::Context::TargetMemoryPoolAlignment)) char[image_memory_size];
        DD_ASSERT(memory_buffer != nullptr);
        DD_ASSERT(memory_image != nullptr);
        
        ::memcpy(memory_buffer, vertices, sizeof(vertices));
        ::memcpy(reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(memory_buffer) + index_buffer_info.offset), indices, sizeof(indices));

        ViewArg view_arg[CubeCount] = {};
        for (u32 i = 0; i < CubeCount; ++i) {
            view_arg[i].model_matrix = model_matrix;
            view_arg[i].view_matrix = *camera.GetCameraMatrix();
            view_arg[i].projection_matrix = *perspective_projection.GetProjectionMatrix();
            
            float rot_angle = 20.0f * i;
            dd::util::math::RotateLocalX(std::addressof(view_arg[i].model_matrix), rot_angle);
            dd::util::math::RotateLocalY(std::addressof(view_arg[i].model_matrix), 0.3f * rot_angle);
            dd::util::math::RotateLocalZ(std::addressof(view_arg[i].model_matrix), 0.5f * rot_angle);
        }
        ::memcpy(reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(memory_buffer) + uniform_buffer_info.offset), view_arg, sizeof(view_arg));
        
        ::memcpy(memory_image, texture0, texture0_size);
        ::memcpy(reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(memory_image) + texture1_info.memory_offset), texture1, texture1_size);

        /* Free unneeded image buffers */
        res::FreeStbImage(texture0);
        res::FreeStbImage(texture1);

        /* Create buffer memory pool */
        util::ConstructAt(vk_buffer_memory);
        const vk::MemoryPoolInfo buffer_pool_info = {
            .size                     = buffer_memory_size,
            .vk_memory_property_flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            .import_memory            = memory_buffer
        };
        vk::MemoryPool *buffer_pool = util::GetPointer(vk_buffer_memory);
        buffer_pool->Initialize(context, std::addressof(buffer_pool_info));
        
        /* Create image memory pool */
        util::ConstructAt(vk_image_memory);
        const vk::MemoryPoolInfo image_pool_info = {
            .size                     = image_memory_size,
            .vk_memory_property_flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            .import_memory            = memory_image
        };
        vk::MemoryPool *image_pool = util::GetPointer(vk_image_memory);
        image_pool->Initialize(context, std::addressof(image_pool_info));

        /* Create buffers */
        util::ConstructAt(vk_vertex_buffer);
        util::GetReference(vk_vertex_buffer).Initialize(context, std::addressof(vertex_buffer_info), buffer_pool);

        util::ConstructAt(vk_index_buffer);
        util::GetReference(vk_index_buffer).Initialize(context, std::addressof(index_buffer_info), buffer_pool);

        util::ConstructAt(vk_uniform_buffer);
        util::GetReference(vk_uniform_buffer).Initialize(context, std::addressof(uniform_buffer_info), buffer_pool);

        /* Create textures */
        util::ConstructAt(vk_texture0);
        util::GetReference(vk_texture0).Initialize(context, std::addressof(texture0_info), image_pool);

        util::ConstructAt(vk_texture1);
        util::GetReference(vk_texture1).Initialize(context, std::addressof(texture1_info), image_pool);

        /* Create texture views */
        vk::TextureViewInfo view_info = {};
        view_info.SetDefaults();

        view_info.vk_format  = VK_FORMAT_R8G8B8A8_UNORM;
        view_info.texture = util::GetPointer(vk_texture0);

        util::ConstructAt(vk_texture_view0);
        util::GetReference(vk_texture_view0).Initialize(context, std::addressof(view_info));

        view_info.vk_format  = VK_FORMAT_R8G8B8A8_UNORM;
        view_info.texture = util::GetPointer(vk_texture1);

        util::ConstructAt(vk_texture_view1);
        util::GetReference(vk_texture_view1).Initialize(context, std::addressof(view_info));

        /* Create sampler */
        vk::SamplerInfo sampler_info = {};
        sampler_info.SetDefaults();

        util::ConstructAt(vk_sampler);
        util::GetReference(vk_sampler).Initialize(context, std::addressof(sampler_info));

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

        /* Create texture descriptor pool */
        vk::DescriptorPool *texture_descriptor_pool = util::GetPointer(vk_texture_descriptor_pool);

        util::ConstructAt(vk_texture_descriptor_pool);
        texture_descriptor_pool->Initialize(context, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 32);

        /* Create sampler descriptor pool */
        vk::DescriptorPool *sampler_descriptor_pool = util::GetPointer(vk_sampler_descriptor_pool);

        util::ConstructAt(vk_sampler_descriptor_pool);
        sampler_descriptor_pool->Initialize(context, VK_DESCRIPTOR_TYPE_SAMPLER, 16);

        /* Register our textures */
        sampler_slot = sampler_descriptor_pool->RegisterSampler(util::GetPointer(vk_sampler));
        texture_view0_slot = util::GetPointer(vk_texture_descriptor_pool)->RegisterTexture(util::GetPointer(vk_texture_view0));
        texture_view1_slot = util::GetPointer(vk_texture_descriptor_pool)->RegisterTexture(util::GetPointer(vk_texture_view1));
    }

    void CalcTriangle() {

        /* Process input */
        u32 width = 0, height = 0;
        vk::GetGlobalContext()->GetWindowDimensions(std::addressof(width), std::addressof(height));
        perspective_projection.SetAspect(static_cast<float>(width) / static_cast<float>(height));

        util::math::Vector3f new_pos = {};
        util::math::Vector3f up = {};

        camera.GetPos(std::addressof(new_pos));
        camera.GetUp(std::addressof(up));
        float speed = 2.5f * util::GetDeltaTime();

        hid::MouseState mouse_state = hid::GetMouseState();

        float sensitivity = 0.1f;

        yaw   += sensitivity * mouse_state.delta_x;
        pitch += sensitivity * mouse_state.delta_y;

        const float yaw_rad   = util::math::AngleHalfRound(util::math::ToRadians(yaw));
        const float pitch_rad = util::math::AngleHalfRound(util::math::ToRadians(pitch));
        const float cos_pitch = util::math::SampleCos(pitch_rad);

        const util::math::Vector3f dir = {
            util::math::SampleCos(yaw_rad) * cos_pitch,
            util::math::SampleSin(pitch_rad),
            util::math::SampleSin(yaw_rad) * cos_pitch
        };

        const util::math::Vector3f front = dir.Normalize();

        hid::KeyboardState key_state = hid::GetKeyboardState();

        if (key_state.IsKeyHeld(hid::VirtualKey_W) == true) {
            new_pos += front * speed;
        }
        if (key_state.IsKeyHeld(hid::VirtualKey_A) == true) {
            new_pos -= front.Cross(up).Normalize() * speed;
        }
        if (key_state.IsKeyHeld(hid::VirtualKey_S) == true) {
            new_pos -= front * speed;
        }
        if (key_state.IsKeyHeld(hid::VirtualKey_D) == true) {
            new_pos += front.Cross(up).Normalize() * speed;
        }

        camera.SetPos(new_pos);
        camera.SetAt(new_pos + front);
    }
    
    void DrawTriangle(vk::CommandBuffer *command_buffer) {

        /* Copy matrices to buffer */
        u32 width = 0, height = 0;
        vk::GetGlobalContext()->GetWindowDimensionsUnsafe(std::addressof(width), std::addressof(height));

        camera.UpdateCameraMatrixSelf();

        ViewArg view_arg[CubeCount] = {};
        for (u32 i = 0; i < CubeCount; ++i) {
            view_arg[i].model_matrix = model_matrix;
            view_arg[i].view_matrix = *camera.GetCameraMatrix();
            view_arg[i].projection_matrix = *perspective_projection.GetProjectionMatrix();

            view_arg[i].model_matrix.SetColumn(3, CubePositions[i]);
            float rot_angle = 20.0f * i;
            dd::util::math::RotateLocalX(std::addressof(view_arg[i].model_matrix), rot_angle);
            dd::util::math::RotateLocalY(std::addressof(view_arg[i].model_matrix), 0.3f * rot_angle);
            dd::util::math::RotateLocalZ(std::addressof(view_arg[i].model_matrix), 0.5f * rot_angle);
        }

        void *ubo_address = util::GetReference(vk_uniform_buffer).Map();
        DD_ASSERT(ubo_address != nullptr);
        ::memcpy(ubo_address, view_arg, sizeof(view_arg));
        util::GetReference(vk_uniform_buffer).Unmap();

        /* Ensure textures are transistioned */
        if (util::GetPointer(vk_texture_view0)->GetTexture()->GetImageLayout() == VK_IMAGE_LAYOUT_UNDEFINED) {

            util::GetReference(vk_image_memory).Relocate(command_buffer->GetCommandBuffer());

            const dd::vk::TextureBarrierCmdState texture_barrier_state = {
                .vk_dst_stage_mask  = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                .vk_dst_access_mask = VK_ACCESS_SHADER_READ_BIT,
                .vk_src_layout      = VK_IMAGE_LAYOUT_UNDEFINED,
                .vk_dst_layout      = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
            };

            command_buffer->SetTextureStateTransition(util::GetPointer(vk_texture_view0)->GetTexture(), std::addressof(texture_barrier_state), VK_IMAGE_ASPECT_COLOR_BIT);
        }
        if (util::GetPointer(vk_texture_view1)->GetTexture()->GetImageLayout() == VK_IMAGE_LAYOUT_UNDEFINED) {

            const dd::vk::TextureBarrierCmdState texture_barrier_state = {
                .vk_dst_stage_mask  = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                .vk_dst_access_mask = VK_ACCESS_SHADER_READ_BIT,
                .vk_src_layout      = VK_IMAGE_LAYOUT_UNDEFINED,
                .vk_dst_layout      = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
            };

            command_buffer->SetTextureStateTransition(util::GetPointer(vk_texture_view1)->GetTexture(), std::addressof(texture_barrier_state), VK_IMAGE_ASPECT_COLOR_BIT);
        }

        /* Bind */
        command_buffer->SetDescriptorPool(util::GetPointer(vk_sampler_descriptor_pool));
        command_buffer->SetDescriptorPool(util::GetPointer(vk_texture_descriptor_pool));
        command_buffer->SetPipeline(util::GetPointer(vk_pipeline));
        command_buffer->SetPipelineState(std::addressof(vk_pipeline_cmd_state));

        command_buffer->SetVertexBuffer(0, util::GetPointer(vk_vertex_buffer), VerticeStride, sizeof(vertices));

        command_buffer->SetUniformBuffer(0, vk::ShaderStage_Vertex, util::GetReference(vk_uniform_buffer).GetGpuAddress());

        command_buffer->SetTextureAndSampler(0, vk::ShaderStage_Fragment, texture_view0_slot, sampler_slot);
        command_buffer->SetTextureAndSampler(1, vk::ShaderStage_Fragment, texture_view1_slot, sampler_slot);

        VkViewport viewport {
            .width  = static_cast<float>(width),
            .height = static_cast<float>(height),
            .minDepth = 1.0f,
            .maxDepth = 0.0f
        };
        command_buffer->SetViewports(1, std::addressof(viewport));

        VkRect2D scissor = {
            .extent = {
                .width = width,
                .height = height
            }
        };
        command_buffer->SetScissors(1, std::addressof(scissor));

        /* Draw */
        command_buffer->DrawInstanced(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VerticeCount, 0, CubeCount, 0);
    }

    void CleanTriangle() {
        vk::Context *context = vk::GetGlobalContext();

        util::GetReference(vk_shader).Finalize(context);
        dd::util::DestructAt(vk_shader);

        util::GetReference(vk_pipeline).Finalize(context);
        dd::util::DestructAt(vk_pipeline);

        util::GetReference(vk_texture_descriptor_pool).UnregisterResourceBySlot(texture_view1_slot);
        util::GetReference(vk_texture_descriptor_pool).UnregisterResourceBySlot(texture_view0_slot);
        util::GetReference(vk_texture_descriptor_pool).Finalize(context);
        dd::util::DestructAt(vk_texture_descriptor_pool);

        util::GetReference(vk_sampler_descriptor_pool).UnregisterResourceBySlot(sampler_slot);
        util::GetReference(vk_sampler_descriptor_pool).Finalize(context);
        dd::util::DestructAt(vk_sampler_descriptor_pool);

        util::GetReference(vk_texture_view0).Finalize(context);
        dd::util::DestructAt(vk_texture_view0);

        util::GetReference(vk_texture_view1).Finalize(context);
        dd::util::DestructAt(vk_texture_view1);

        util::GetReference(vk_texture0).Finalize(context);
        dd::util::DestructAt(vk_texture0);

        util::GetReference(vk_texture1).Finalize(context);
        dd::util::DestructAt(vk_texture1);

        util::GetReference(vk_sampler).Finalize(context);
        dd::util::DestructAt(vk_sampler);

        util::GetReference(vk_uniform_buffer).Finalize(context);
        dd::util::DestructAt(vk_uniform_buffer);

        util::GetReference(vk_index_buffer).Finalize(context);
        dd::util::DestructAt(vk_index_buffer);

        util::GetReference(vk_vertex_buffer).Finalize(context);
        dd::util::DestructAt(vk_vertex_buffer);

        util::GetReference(vk_image_memory).Finalize(context);
        dd::util::DestructAt(vk_image_memory);

        util::GetReference(vk_buffer_memory).Finalize(context);
        dd::util::DestructAt(vk_buffer_memory);
    }
}
