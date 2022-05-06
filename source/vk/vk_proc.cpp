#include <dd.hpp>

#define DEFINE_VK_PROC(name) PFN_##name pfn_##name
#define LOAD_DEVICE_VK_PROC(name) pfn_##name = reinterpret_cast<PFN_##name>(::vkGetDeviceProcAddr(device, #name));
#define LOAD_INSTANCE_VK_PROC(name) pfn_##name = reinterpret_cast<PFN_##name>(::vkGetInstanceProcAddr(instance, #name));

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
    
    void LoadCProcsDevice(VkDevice device) {
        LOAD_DEVICE_VK_PROC(vkGetMemoryHostPointerPropertiesEXT);
    }
}