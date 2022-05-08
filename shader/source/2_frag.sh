#version 460 core
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


#include "resource_buffer.shinc"

#if defined (DD_VERTEX_SHADER)

    layout (row_major, buffer_reference) buffer VertexUbo {
        mat4x3 uTransform;
    };

    struct ResourceBuffer {
        VertexUbo ubo;
        ADDRESS_PADDING(29);
        TEXTURE_PADDING(32);
        SAMPLER_PADDING(32);
    };
    
    DECLARE_RESOURCE_BUFFER(ResourceBuffer);

    layout (location = 0) in vec3 aPosition;
    layout (location = 1) in vec3 aColor;
    layout (location = 2) in vec2 aTexCoord;
    
    layout (location = 0) out vec3 ovColor;
    layout (location = 1) out vec2 ovTexCoord;

    void main() {
        
        gl_PointSize = 1.0;
        gl_Position = vec4(rb[resource_index].ubo.uTransform * vec4(aPosition, 1.0), 1.0);
        ovColor = aColor;
        ovTexCoord = aTexCoord;
    }

#endif

#if defined(DD_FRAGMENT_SHADER)
    
    DECLARE_DEFAULT_RESOURCE_BUFFER;

    layout(location = 0) in vec3 ovColor;
    layout(location = 1) in vec2 ovTexCoord;

    layout(location = 0) out vec4 oFragmentColor;

    void main() {
        //oFragmentColor = mix(texture(sampler2D(texture2DIndex(0), samplerIndex(0)), ovTexCoord), texture(sampler2D(texture2DIndex(1), samplerIndex(0)), ovTexCoord), 0.2);
        oFragmentColor = texture(SAMPLER2D(0), ovTexCoord);
        //oFragmentColor = vec4(ovColor,1.0);
    }

#endif
