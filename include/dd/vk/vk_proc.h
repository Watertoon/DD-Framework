#pragma once

#define DEFINE_EXTERN_VK_PROC(name) extern PFN_##name pfn_##name

namespace dd::vk {
    
    /* Instance procs */
    DEFINE_EXTERN_VK_PROC(vkCreateDebugUtilsMessengerEXT);
    DEFINE_EXTERN_VK_PROC(vkDestroyDebugUtilsMessengerEXT);

    void LoadCProcsInstance(VkInstance instance);

    /* Device procs */
    DEFINE_EXTERN_VK_PROC(vkGetMemoryHostPointerPropertiesEXT);

    void LoadCProcsDevice(VkDevice device);
}