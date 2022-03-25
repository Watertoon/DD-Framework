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
#version 460 core

in vec3 ovColor;
in vec2 ovTexCoord;

uniform sampler2D uDiffuse0;
uniform sampler2D uDiffuse1;

out vec4 oFragmentColor;

void main() {
    oFragmentColor = mix(texture(uDiffuse0, ovTexCoord), texture(uDiffuse1, ovTexCoord), 0.2);
}