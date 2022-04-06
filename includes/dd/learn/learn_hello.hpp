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

namespace dd::learn {
    namespace {
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
        u32 vbo_index, vao_index, ibo_index;
        ogl::Shader shader;
        ogl::Texture texture0;
        ogl::Texture texture1;
        dd::util::math::Matrix34f transform(0.5f, 0.0f, 0.0f, 0.5f, 
                                            0.0f, 1.5f, 0.0f, -0.3f,
                                            0.0f, 0.0f, 1.0f, 0.0f);
    }

    void SetupTriangle() {
        /* Load Shader */
        char *vertex_shader = nullptr;
        char *fragment_shader = nullptr;
        res::LoadTextFile(vertex_shader_path, std::addressof(vertex_shader), nullptr);
        res::LoadTextFile(fragment_shader_path, std::addressof(fragment_shader), nullptr);

        /* Create Shader */
        shader.AddShaderStage(vertex_shader, ogl::Shader::ShaderStage_Vertex);
        shader.AddShaderStage(fragment_shader, ogl::Shader::ShaderStage_Fragment);
        shader.CompileShader();

        delete[](vertex_shader);
        delete[](fragment_shader);
        
        /* Create Textures */
        {
            unsigned char *image_buffer = nullptr;
            s32 width = 0, height = 0, channels = 0;
            res::LoadStbImage("resources/woodcrate.jpg", 0, std::addressof(image_buffer), std::addressof(width), std::addressof(height), std::addressof(channels));
            const ogl::TextureCreateArg texture_arg = {
                .image_buffer = image_buffer,
                .width = width,
                .height = height,
                .channels = GL_RGB
            };
            texture0.InitializeTexture(std::addressof(texture_arg));
            res::FreeStbImage(image_buffer);
        }
        {
            unsigned char *image_buffer = nullptr;
            s32 width = 0, height = 0, channels = 0;
            res::LoadStbImage("resources/awesomeface.png", 0, std::addressof(image_buffer), std::addressof(width), std::addressof(height), std::addressof(channels));
            const ogl::TextureCreateArg texture_arg = {
                .image_buffer = image_buffer,
                .width = width,
                .height = height,
                .channels = GL_RGBA
            };
            texture1.InitializeTexture(std::addressof(texture_arg));
            res::FreeStbImage(image_buffer);
        }

        shader.SetUniform("uDiffuse0", 0);
        shader.SetUniform("uDiffuse1", 1);

        /* Create Vertex Array */
        (pfn_glGenVertexArrays)(1, std::addressof(vao_index));
        (pfn_glBindVertexArray)(vao_index);

        /* Create Vertex Buffer */
        (pfn_glGenBuffers)(1, std::addressof(vbo_index));
        (pfn_glBindBuffer)(GL_ARRAY_BUFFER, vbo_index);
        (pfn_glBufferData)(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        /* Create Index Buffer */
        (pfn_glGenBuffers)(1, std::addressof(ibo_index));
        (pfn_glBindBuffer)(GL_ELEMENT_ARRAY_BUFFER, ibo_index);
        (pfn_glBufferData)(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        /* Set vertex attribute */
        (pfn_glVertexAttribPointer)(0, 3, GL_FLOAT, GL_FALSE, VerticeStride, nullptr);
        (pfn_glVertexAttribPointer)(1, 3, GL_FLOAT, GL_FALSE, VerticeStride, reinterpret_cast<void*>(3 * sizeof(float)));
        (pfn_glVertexAttribPointer)(2, 2, GL_FLOAT, GL_FALSE, VerticeStride, reinterpret_cast<void*>(6 * sizeof(float)));
        (pfn_glEnableVertexAttribArray)(0);
        (pfn_glEnableVertexAttribArray)(1);
        (pfn_glEnableVertexAttribArray)(2);
    }

    void DrawTriangle() {
        /* Clear last frame */
        (pfn_glClearColor)(0.2f, 0.3f, 0.3f, 1.0f);
        (pfn_glClear)(GL_COLOR_BUFFER_BIT);

        /* Bind texture resources */
        (pfn_glActiveTexture)(GL_TEXTURE0);
        texture0.BindTexture();
        (pfn_glActiveTexture)(GL_TEXTURE1);
        texture1.BindTexture();

        /* Bind shader resources */
        shader.BindShader();

        /* Create rotation */
        dd::util::math::RotateLocalZ(std::addressof(transform), dd::util::math::TRadians<float, 1.0f>);
        shader.SetUniformMatrix("uTransform", transform.m_arr);

        /* Bind vertex array objects */
        (pfn_glBindVertexArray)(vao_index);

        /* Draw */
        (pfn_glDrawElements)(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }

    void CleanTriangle() {
        (pfn_glDeleteVertexArrays)(1, std::addressof(vao_index));
        (pfn_glDeleteBuffers)(1, std::addressof(vbo_index));
        (pfn_glDeleteBuffers)(1, std::addressof(ibo_index));
        shader.FinalizeShader();
    }
}