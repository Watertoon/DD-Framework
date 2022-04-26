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

/* Libc */
#include <cmath>

/* STD */
#include <iostream>
#include <memory>
#include <type_traits>

/* Windows */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

/* OpenGL */
#include <gl/glcorearb.h>
#include <gl/gl.h>
#include <gl/glext.h>
#include <gl/wgl.h>
#include <gl/wglext.h>

/* Vulkan */
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

/* DD */
#include "util_defines.h"