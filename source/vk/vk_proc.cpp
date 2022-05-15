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

#define DEFINE_VK_PROC(name) PFN_##name pfn_##name
#define LOAD_DEVICE_VK_PROC(name) pfn_##name = reinterpret_cast<PFN_##name>(::vkGetDeviceProcAddr(device, #name)); DD_ASSERT(pfn_##name != nullptr);
#define LOAD_INSTANCE_VK_PROC(name) pfn_##name = reinterpret_cast<PFN_##name>(::vkGetInstanceProcAddr(instance, #name)); DD_ASSERT(pfn_##name != nullptr);

namespace dd::vk {
    
    /* Instance procs */
    DEFINE_VK_PROC(vkCreateDebugUtilsMessengerEXT);
    DEFINE_VK_PROC(vkDestroyDebugUtilsMessengerEXT);

    void LoadCProcsInstance(VkInstance instance) {
        LOAD_INSTANCE_VK_PROC(vkCreateDebugUtilsMessengerEXT);
        LOAD_INSTANCE_VK_PROC(vkDestroyDebugUtilsMessengerEXT);
    }
    
    /* Device procs */
    
    DEFINE_VK_PROC(vkGetMemoryHostPointerPropertiesEXT);
    DEFINE_VK_PROC(vkCmdSetLogicOpEXT);
    DEFINE_VK_PROC(vkCmdSetVertexInputEXT);
    
    void LoadCProcsDevice(VkDevice device) {
        LOAD_DEVICE_VK_PROC(vkGetMemoryHostPointerPropertiesEXT);
        LOAD_DEVICE_VK_PROC(vkCmdSetLogicOpEXT);
        LOAD_DEVICE_VK_PROC(vkCmdSetVertexInputEXT);
    }
}
