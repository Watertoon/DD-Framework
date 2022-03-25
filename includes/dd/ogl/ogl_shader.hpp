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

namespace dd::ogl {

    class Shader {
        public:
            static constexpr inline u32 MaxShaderStages = 2;
        public:
            enum ShaderStage {
                ShaderStage_Vertex = 0,
                ShaderStage_Fragment = 1
            };
        private:
            u32   m_program_index;
            const char *m_code_array[MaxShaderStages];
        public:
            constexpr Shader() { /*...*/ }

            constexpr ~Shader() {
                if (!std::is_constant_evaluated()) {
                    if (m_program_index != 0) {
                        (pfn_glDeleteShader)(m_program_index);
                    }
                }
            }

            void AddShaderStage(const char *shader_code, ShaderStage stage) {
                m_code_array[stage] = shader_code;
            }

            void CompileShader() {
                u32 v_shader_index = 0, f_shader_index = 0;
                s32 success = 0;
                m_program_index = (pfn_glCreateProgram)();

                if (m_code_array[ShaderStage_Vertex] != nullptr) {
                    /* Compile Shader*/
                    v_shader_index = (pfn_glCreateShader)(GL_VERTEX_SHADER);
                    (pfn_glShaderSource)(v_shader_index, 1, std::addressof(m_code_array[ShaderStage_Vertex]), nullptr);
                    (pfn_glCompileShader)(v_shader_index);
                    success = 0;
                    (pfn_glGetShaderiv)(v_shader_index, GL_COMPILE_STATUS, std::addressof(success));
                    DD_ASSERT(success == true);
    
                    /* Add to Program */
                    (pfn_glAttachShader)(m_program_index, v_shader_index);
                }

                if (m_code_array[ShaderStage_Fragment] != nullptr) {
                    /* Compile Shader*/
                    f_shader_index = (pfn_glCreateShader)(GL_FRAGMENT_SHADER);
                    (pfn_glShaderSource)(f_shader_index, 1, std::addressof(m_code_array[ShaderStage_Fragment]), nullptr);
                    (pfn_glCompileShader)(f_shader_index);
                    success = 0;
                    (pfn_glGetShaderiv)(f_shader_index, GL_COMPILE_STATUS, std::addressof(success));
                    DD_ASSERT(success == true);
    
                    /* Add to Program */
                    (pfn_glAttachShader)(m_program_index, f_shader_index);
                }

                /* Compile Shader */
                (pfn_glLinkProgram)(m_program_index);
                success = 0;
                (pfn_glGetProgramiv)(m_program_index, GL_LINK_STATUS, std::addressof(success));
                DD_ASSERT(success == true);

                /* Delete intermediate shader objects */
                if (v_shader_index != 0) {
                    (pfn_glDeleteShader)(v_shader_index);
                }
                if (f_shader_index != 0) {
                    (pfn_glDeleteShader)(f_shader_index);
                }
            }

            void BindShader() {
                (pfn_glUseProgram)(m_program_index);
            }

            void FinalizeShader() {
                (pfn_glDeleteShader)(m_program_index);
                m_program_index = 0;
            }

            template<typename T>
            void SetUniform(const char *name, const T& value) {
                /* Find required OpenGL function at compile time */
                if constexpr (std::is_floating_point<T>::value == true) {
                    /* Floating point scalars */
                    if constexpr (sizeof(T) == sizeof(float)) {
                        (pfn_glProgramUniform1fv)(m_program_index, (pfn_glGetUniformLocation)(m_program_index, name), 1, std::addressof(value));
                    } else if constexpr (sizeof(T) == sizeof(double)) {
                        (pfn_glProgramUniform1dv)(m_program_index, (pfn_glGetUniformLocation)(m_program_index, name), 1, std::addressof(value));
                    }
                } else if constexpr (std::is_integral<T>::value == true) {
                    /* Integral scalars */
                    if constexpr (std::is_unsigned<T>::value == true) {
                        if constexpr (sizeof(T) == sizeof(unsigned int)) {
                            (pfn_glProgramUniform1uiv)(m_program_index, (pfn_glGetUniformLocation)(m_program_index, name), 1, std::addressof(value));
                        }
                    } else if constexpr (std::is_signed<T>::value == true) {
                        if constexpr (sizeof(T) == sizeof(int)) {
                            (pfn_glProgramUniform1iv)(m_program_index, (pfn_glGetUniformLocation)(m_program_index, name), 1, std::addressof(value));
                        }
                    }
                } else if constexpr (std::is_array<T>::value == true) {
                    /* Vectors (seperate due to array type deduction) */
                    if constexpr (std::is_floating_point<dd::util::element_type<T>>::value == true) {
                        /* Floating point Vectors */
                        if constexpr (sizeof(dd::util::element_type<T>) == sizeof(float)) {
                            if constexpr (sizeof(T) / sizeof(float) == 2) {
                                (pfn_glProgramUniform2fv)(m_program_index, (pfn_glGetUniformLocation)(m_program_index, name), 1, value);
                            } else if constexpr (sizeof(T) / sizeof(float) == 3) {
                                (pfn_glProgramUniform3fv)(m_program_index, (pfn_glGetUniformLocation)(m_program_index, name), 1, value);
                            } else if constexpr (sizeof(T) / sizeof(float) == 4) {
                                (pfn_glProgramUniform4fv)(m_program_index, (pfn_glGetUniformLocation)(m_program_index, name), 1, value);
                            }
                        } else if (sizeof(dd::util::element_type<T>) == sizeof(double)) {
                            if constexpr (sizeof(T) / sizeof(double) == 2) {
                                (pfn_glProgramUniform2dv)(m_program_index, (pfn_glGetUniformLocation)(m_program_index, name), 1, value);
                            } else if constexpr (sizeof(T) / sizeof(double) == 3) {
                                (pfn_glProgramUniform3dv)(m_program_index, (pfn_glGetUniformLocation)(m_program_index, name), 1, value);
                            } else if constexpr (sizeof(T) / sizeof(double) == 4) {
                                (pfn_glProgramUniform4dv)(m_program_index, (pfn_glGetUniformLocation)(m_program_index, name), 1, value);
                            }
                        }
                    } else if constexpr (std::is_integral<dd::util::element_type<T>>::value == true) {
                        /* Integral Vectors */
                        if constexpr (std::is_unsigned<dd::util::element_type<T>>::value == true) {
                            if constexpr (sizeof(T) / sizeof(unsigned int) == 2) {
                                (pfn_glProgramUniform2uiv)(m_program_index, (pfn_glGetUniformLocation)(m_program_index, name), 1, value);
                            } else if constexpr (sizeof(T) / sizeof(unsigned int) == 3) {
                                (pfn_glProgramUniform3uiv)(m_program_index, (pfn_glGetUniformLocation)(m_program_index, name), 1, value);
                            } else if constexpr (sizeof(T) / sizeof(unsigned int) == 4) {
                                (pfn_glProgramUniform4uiv)(m_program_index, (pfn_glGetUniformLocation)(m_program_index, name), 1, value);
                            }
                        } else if constexpr (std::is_signed<dd::util::element_type<T>>::value == true) {
                            if constexpr (sizeof(T) / sizeof(int) == 2) {
                                (pfn_glProgramUniform2iv)(m_program_index, (pfn_glGetUniformLocation)(m_program_index, name), 1, value);
                            } else if constexpr (sizeof(T) / sizeof(int) == 3) {
                                (pfn_glProgramUniform3iv)(m_program_index, (pfn_glGetUniformLocation)(m_program_index, name), 1, value);
                            } else if constexpr (sizeof(T) / sizeof(int) == 4) {
                                (pfn_glProgramUniform4iv)(m_program_index, (pfn_glGetUniformLocation)(m_program_index, name), 1, value);
                            }
                        }
                    }
                }
            }

            constexpr u32 GetNativeHandle() const { return m_program_index; }
    };
}