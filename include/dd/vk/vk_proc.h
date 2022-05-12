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

#define DEFINE_EXTERN_VK_PROC(name) extern PFN_##name pfn_##name

namespace dd::vk {
    
    /* Instance procs */
    DEFINE_EXTERN_VK_PROC(vkCreateDebugUtilsMessengerEXT);
    DEFINE_EXTERN_VK_PROC(vkDestroyDebugUtilsMessengerEXT);

    void LoadCProcsInstance(VkInstance instance);

    /* Device procs */
    DEFINE_EXTERN_VK_PROC(vkGetMemoryHostPointerPropertiesEXT);
    DEFINE_EXTERN_VK_PROC(vkCmdSetLogicOpEXT);
    DEFINE_EXTERN_VK_PROC(vkCmdSetVertexInputEXT);

    void LoadCProcsDevice(VkDevice device);
}
