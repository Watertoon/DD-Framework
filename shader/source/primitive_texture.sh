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
#extension GL_EXT_debug_printf : enable

struct Transform {
    mat4x3 model;
    mat4x3 view;
    mat4   proj;
};

struct Light {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

#if defined (DD_VERTEX_SHADER)

    layout (row_major, scalar, buffer_reference) buffer VertexUbo {
        TransformMatrices matrices[10];
        Light             light;
    };

    struct ResourceBuffer {
        VertexUbo ubo;
        ADDRESS_PADDING(29);
        TEXTURE_PADDING;
        SAMPLER_PADDING;
        uint reserve[4];
    };

    DECLARE_RESOURCE_BUFFER(ResourceBuffer);

    layout (location = 0) in vec3 aPosition;
    layout (location = 1) in vec3 aNormal;
    layout (location = 2) in vec2 aTexCoord;
    
    layout (location = 0) out vec3 oNormal;
    layout (location = 1) out vec2 oTexCoord;

    void main() {
        //debugPrintfEXT("view :\n");
        //debugPrintfEXT("%vf3 \n", RB(ubo.matrices[gl_InstanceIndex].view[0]));
        //debugPrintfEXT("%vf3 \n", RB(ubo.matrices[gl_InstanceIndex].view[1]));
        //debugPrintfEXT("%vf3 \n", RB(ubo.matrices[gl_InstanceIndex].view[2]));
        gl_Position = RB(ubo.matrices[gl_InstanceIndex].proj) * mat4(RB(ubo.matrices[gl_InstanceIndex].view)) * mat4(RB(ubo.matrices[gl_InstanceIndex].model)) * vec4(aPosition, 1.0);
        oNormal = aNormal;
        oTexCoord = aTexCoord;
    }

#endif

#if defined(DD_FRAGMENT_SHADER)

    layout (row_major, scalar, buffer_reference) buffer FragmentUbo {
        TransformMatrices matrices[10];
        Light             light;
    };

    struct ResourceBuffer {
        FragmentUbo ubo;
        ADDRESS_PADDING(29);
        TEXTURE_PADDING;
        SAMPLER_PADDING;
        uint reserve[4];
    };

    DECLARE_RESOURCE_BUFFER(ResourceBuffer);

    layout(location = 0) in vec3 aNormal;
    layout(location = 1) in vec2 aTexCoord;

    layout(location = 0) out vec4 oFragmentColor;

    void main() {
        /* Sampler2d */
        uint a0 = 0;
        uint s0 = 1;
    
        vec3 ambient  = light.ambient  * vec3(texture(SAMPLER2D(a0), aTexCoord));
        vec3 diffuse  = light.diffuse  * vec3(texture(SAMPLER2D(a0), aTexCoord));
        vec3 specular = light.specular * vec3(texture(SAMPLER2D(s0), aTexCoord));
        oFragmentColor = vec4(ambient + diffuse + specular, 1.0);
    }

#endif
