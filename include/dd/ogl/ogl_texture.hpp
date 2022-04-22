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

    struct TextureCreateArg {
        unsigned char *image_buffer;
        s32 width;
        s32 height;
        s32 channels;
    };

    class Texture {
        private:
            u32 m_texture_index;
        public:
            constexpr Texture() { /*...*/ }

            void InitializeTexture(const TextureCreateArg* create_arg) {
                /* Create Texture */
                (pfn_glGenTextures)(1, std::addressof(m_texture_index));
                (pfn_glBindTexture)(GL_TEXTURE_2D, m_texture_index);

                /* Set texture options */
                (pfn_glTexParameteri)(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                (pfn_glTexParameteri)(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                (pfn_glTexParameteri)(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                (pfn_glTexParameteri)(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

                /* Set image data */
                (pfn_glTexImage2D)(GL_TEXTURE_2D, 0, GL_RGB, create_arg->width, create_arg->height, 0, create_arg->channels, GL_UNSIGNED_BYTE, create_arg->image_buffer);
                (pfn_glGenerateMipmap)(GL_TEXTURE_2D);
            }

            void FinalizeTexture() {
                (pfn_glDeleteTextures)(1, std::addressof(m_texture_index));
            }

            void BindTexture() {
                (pfn_glBindTexture)(GL_TEXTURE_2D, m_texture_index);
            }
    };
}