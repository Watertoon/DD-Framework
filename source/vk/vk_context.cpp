#include <dd.hpp>

namespace dd::vk {
    
    namespace {
        
        #if defined(DD_DEBUG)
            constexpr const char *DebugLayers = {
                "VK_LAYER_KHRONOS_validation"
            };
            constexpr u32 DebugLayerCount = sizeof(DebugLayers) / sizeof(const char*);
        #endif
        
        constexpr const char *InstanceExtensions[] = {
            #if defined(DD_DEBUG)
                VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
            #endif
            VK_KHR_SURFACE_EXTENSION_NAME,
            VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME,
            VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        };
        constexpr u32 InstanceExtensionCount = sizeof(InstanceExtensions) / sizeof(const char*);

        constexpr const char *DeviceExtensions[] = {
            VK_KHR_SHARED_PRESENTABLE_IMAGE_EXTENSION_NAME,
            VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME,
            VK_EXT_VERTEX_INPUT_DYNAMIC_STATE_EXTENSION_NAME
        };
        constexpr u32 DeviceExtensionCount = sizeof(DeviceExtensions) / sizeof(const char*);
        
        static VkPhysicalDeviceVulkan13Features TargetDeviceFeatures13 = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES
        };
        static VkPhysicalDeviceVulkan12Features TargetDeviceFeatures12 = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
            .pNext = std::addressof(TargetDeviceFeatures13),
            .descriptorIndexing = true,
            .bufferDeviceAddress = true
        };
        static VkPhysicalDeviceVulkan11Features TargetDeviceFeatures11 = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
            .pNext = std::addressof(TargetDeviceFeatures12)
        };
        static VkPhysicalDeviceFeatures2 TargetDeviceFeatures = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
            .pNext = std::addressof(TargetDeviceFeatures11),
            .features = {
                .independentBlend = true,
                .geometryShader = true,
                .tessellationShader = true,
                .logicOp = true,
                .depthClamp = true,
                .depthBiasClamp = true,
                .fillModeNonSolid = true,
                .depthBounds = true,
                .wideLines = true,
                .largePoints = true,
                .multiViewport = true,
                .samplerAnisotropy = true,
                .textureCompressionASTC_LDR = true,
                .textureCompressionBC = true,
                .fragmentStoresAndAtomics = true,
                .shaderStorageImageExtendedFormats = true
            }
        };
        
        static LRESULT CALLBACK WndProc(HWND window_handle, UINT message, WPARAM w_param, LPARAM l_param) {
                if (message == WM_DESTROY) {
                    ::PostQuitMessage(0);
                    return 0;
                } else if (message == WM_PAINT) {
                    return 0;
                } else if (message == WM_INPUT) {
                    dd::hid::SetLastRawInput(reinterpret_cast<HRAWINPUT>(l_param));
                }

                return ::DefWindowProc(window_handle, message, w_param, l_param);
            }
    }

    bool Context::PickValidPhysicalDevice() {
        /* Hookup physical device features chain */
        m_vk_physical_device_supported_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        m_vk_physical_device_supported_features.pNext = std::addressof(m_vk_physical_device_supported_features_11);
        m_vk_physical_device_supported_features_11.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
        m_vk_physical_device_supported_features_11.pNext = std::addressof(m_vk_physical_device_supported_features_12);
        m_vk_physical_device_supported_features_12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
        m_vk_physical_device_supported_features_12.pNext = std::addressof(m_vk_physical_device_supported_features_13);
        m_vk_physical_device_supported_features_13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
        
        for (u32 i = 0; i < m_vk_physical_device_count; ++i) {
            /* Query Physical Device */
            ::vkGetPhysicalDeviceProperties2(m_vk_physical_device_array[i], std::addressof(m_vk_physical_device_properties));
            ::vkGetPhysicalDeviceFeatures2(m_vk_physical_device_array[i], std::addressof(m_vk_physical_device_supported_features));

            /*  Ensure Vulkan 1.3 support */
            if (!(TargetMinimumApiVersion <= m_vk_physical_device_properties.properties.apiVersion)) { continue; }

            /* Ensure present support */
            {
                const u32 result = ::vkGetPhysicalDeviceWin32PresentationSupportKHR(m_vk_physical_device, m_graphics_queue_family_index);
                DD_ASSERT(result == VK_SUCCESS);
            }

            /* Ensure support for targeted surface format */
            {
                u32 surface_format_count = 0;
                const u32 result0 = ::vkGetPhysicalDeviceSurfaceFormatsKHR(m_vk_physical_device, m_vk_surface, std::addressof(surface_format_count), nullptr);
                DD_ASSERT(result0 == VK_SUCCESS);

                VkSurfaceFormatKHR *surface_formats = new VkSurfaceFormatKHR[surface_format_count];
                const u32 result1 = ::vkGetPhysicalDeviceSurfaceFormatsKHR(m_vk_physical_device, m_vk_surface, std::addressof(surface_format_count), surface_formats);
                DD_ASSERT(result1 == VK_SUCCESS);

                for (u32 i = 0; i < surface_format_count; ++i) {
                    if (surface_formats[i].format == TargetSurfaceFormat.format && surface_formats[i].colorSpace == TargetSurfaceFormat.colorSpace) {
                        delete [] surface_formats;
                        surface_formats = nullptr;
                        break;
                    }
                }
                if (surface_formats != nullptr) {
                    delete [] surface_formats;
                    continue;
                }
            }

            /* Ensure support for targeted present mode */
            {
                u32 present_mode_count = 0;
                const u32 result0 = ::vkGetPhysicalDeviceSurfacePresentModesKHR(m_vk_physical_device, m_vk_surface, std::addressof(present_mode_count), nullptr);
                DD_ASSERT(result0 == VK_SUCCESS);

                VkPresentModeKHR *present_modes = new VkPresentModeKHR[present_mode_count];
                const u32 result1 = ::vkGetPhysicalDeviceSurfacePresentModesKHR(m_vk_physical_device, m_vk_surface, std::addressof(present_mode_count), present_modes);
                DD_ASSERT(result1 == VK_SUCCESS);

                for (u32 i = 0; i < present_mode_count; ++i) {
                    if (present_modes[i] == TargetPresentMode) {
                        delete [] present_modes;
                        present_modes = nullptr;
                        break;
                    }
                }
                if (present_modes != nullptr) {
                    delete [] present_modes;
                    continue;
                }
            }

            /* Ensure our memory alignment is compatible with out TargetMemoryAlignment */
            {
                const size_t vk_alignment = m_vk_physical_device_properties.properties.limits.minMemoryMapAlignment;
                if (TargetMemoryPoolAlignment % vk_alignment != 0)   { continue; }
                if (!(TargetMemoryPoolAlignment / vk_alignment > 0)) { continue; }
            }

            /* Misc Feature checks */
            if (m_vk_physical_device_supported_features.features.independentBlend == false)             { continue; }
            if (m_vk_physical_device_supported_features.features.geometryShader == false)               { continue; }
            if (m_vk_physical_device_supported_features.features.tessellationShader == false)           { continue; }
            if (m_vk_physical_device_supported_features.features.logicOp == false)                      { continue; }
            if (m_vk_physical_device_supported_features.features.depthClamp == false)                   { continue; }
            if (m_vk_physical_device_supported_features.features.depthBiasClamp == false)               { continue; }
            if (m_vk_physical_device_supported_features.features.fillModeNonSolid == false)             { continue; }
            if (m_vk_physical_device_supported_features.features.depthBounds == false)                  { continue; }
            if (m_vk_physical_device_supported_features.features.wideLines == false)                    { continue; }
            if (m_vk_physical_device_supported_features.features.largePoints == false)                  { continue; }
            if (m_vk_physical_device_supported_features.features.multiViewport == false)                { continue; }
            if (m_vk_physical_device_supported_features.features.samplerAnisotropy == false)            { continue; }
            if (m_vk_physical_device_supported_features.features.textureCompressionASTC_LDR == false)   { continue; }
            if (m_vk_physical_device_supported_features.features.textureCompressionBC == false)         { continue; }
            if (m_vk_physical_device_supported_features.features.fragmentStoresAndAtomics == false)         { continue; }
            if (m_vk_physical_device_supported_features.features.shaderStorageImageExtendedFormats == false)         { continue; }
            

            m_vk_physical_device = m_vk_physical_device_array[i];
            m_physical_device_index = i;
            return true;
        }

        return false;;
    }

    bool Context::FindGraphicsQueueFamily(u32 *queue_family_index) {
        u32 queue_family_count = 0;
        ::vkGetPhysicalDeviceQueueFamilyProperties(m_vk_physical_device, std::addressof(queue_family_count), nullptr);

        VkQueueFamilyProperties *queue_properties = new VkQueueFamilyProperties[queue_family_count];
        ::vkGetPhysicalDeviceQueueFamilyProperties(m_vk_physical_device, std::addressof(queue_family_count), queue_properties);

        for (u32 i = 0; i < queue_family_count; ++i) {
            
            /* Every required graphics queue family feature should be checked here */
            if (!((queue_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != VK_QUEUE_GRAPHICS_BIT)) { continue; }
            
            delete[] queue_properties;
            *queue_family_index = i;
            return true;
        }

        delete[] queue_properties;
        return false;
    }

    

    Context::Context() {
        /* Check supported Vulkan Api version */
        u32 api_version = 0;
        ::vkEnumerateInstanceVersion(std::addressof(api_version));
        DD_ASSERT(TargetMinimumApiVersion <= api_version);

        /* Create instance */
        const VkApplicationInfo app_info = {
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pApplicationName = "Learn",
            .applicationVersion = 1,
            .pEngineName = "DD",
            .engineVersion = 1,
            .apiVersion = TargetMinimumApiVersion
        };

        const VkInstanceCreateInfo instance_info = {
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pApplicationInfo = std::addressof(app_info),
            #if defined(DD_DEBUG)
                .enabledLayerCount = DebugLayerCount,
                .ppEnabledLayerNames = DebugLayers,
            #endif
            .enabledExtensionCount = InstanceExtensionCount,
            .ppEnabledExtensionNames = InstanceExtensions
        };

        u32 result = ::vkCreateInstance(std::addressof(instance_info), nullptr, std::addressof(m_vk_instance));
        DD_ASSERT(result == VK_SUCCESS);

        #if defined(DD_DEBUG)
            /* Create debug messenger */
            const VkDebugUtilsMessengerCreateInfoEXT debug_messenger_info = {
                .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
                .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
                .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
                .pfnUserCallback = DebugCallback,
                .pUserData = nullptr
            };

            result = ::vkCreateDebugUtilsMessengerEXT(m_vk_instance, std::addressof(debug_messenger_info), nullptr, std::addressof(m_debug_messenger));
            DD_ASSERT(result == VK_SUCCESS);
        #endif
        
        /* Create Window */
        const HINSTANCE process_handle = ::GetModuleHandle(nullptr);
        const WNDCLASS wc = {
            .style = CS_OWNDC,
            .lpfnWndProc = WndProc,
            .hInstance = process_handle,
            .hbrBackground = (HBRUSH)(COLOR_BACKGROUND),
            .lpszClassName = "VkDDWindow"
        };
        const u32 result0 = ::RegisterClass(std::addressof(wc));
        DD_ASSERT(result0 != 0);

        m_hwnd = ::CreateWindow("VkDDWindow", "Window", WS_OVERLAPPEDWINDOW, 0, 0, 1280, 720, 0, 0, ::GetModuleHandle(nullptr), 0);
        DD_ASSERT(m_hwnd != nullptr);

        /* Create Surface */
        const VkWin32SurfaceCreateInfoKHR win32_info = {
            .sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
            .hinstance = process_handle,
            .hwnd = m_hwnd
        };

        const u32 result1 = ::vkCreateWin32SurfaceKHR(m_vk_instance, std::addressof(win32_info), nullptr, std::addressof(m_vk_surface));
        DD_ASSERT(result1 == VK_SUCCESS);

        /* Enumerate Physical Devices into list */
        const u32 result2 = ::vkEnumeratePhysicalDevices(m_vk_instance, std::addressof(m_vk_physical_device_count), nullptr);
        DD_ASSERT(result2 == VK_SUCCESS);

        m_vk_physical_device_array = new VkPhysicalDevice[m_vk_physical_device_count];
        DD_ASSERT(m_vk_physical_device_array != nullptr);

        const u32 result3 = ::vkEnumeratePhysicalDevices(m_vk_instance, std::addressof(m_vk_physical_device_count), m_vk_physical_device_array);
        DD_ASSERT(result3 == VK_SUCCESS);

        /* Pick our first valid Physical Device */
        bool b_result0 = this->PickValidPhysicalDevice();
        DD_ASSERT(b_result0 == true);

        /* Obtain a valid graphics queue family */
        u32 graphics_queue_family_index = 0;
        bool b_result1 = this->FindGraphicsQueueFamily(std::addressof(graphics_queue_family_index));
        DD_ASSERT(b_result1 == true);

        /* Create Logical Device */
        const float priority = 1.0f;
        const VkDeviceQueueCreateInfo device_queue_info {
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = graphics_queue_family_index,
            .queueCount = 1,
            .pQueuePriorities = std::addressof(priority)
        };

        const VkDeviceCreateInfo device_info = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .pNext = std::addressof(TargetDeviceFeatures),
            .queueCreateInfoCount = 1,
            .pQueueCreateInfos = std::addressof(device_queue_info),
            .enabledExtensionCount = DeviceExtensionCount,
            .ppEnabledExtensionNames = DeviceExtensions,
            .pEnabledFeatures = nullptr
        };

        result = ::vkCreateDevice(m_vk_physical_device, std::addressof(device_info), nullptr, std::addressof(m_vk_device));
        DD_ASSERT(result == VK_SUCCESS);

        /* Obtain graphics queue */
        ::vkGetDeviceQueue(m_vk_device, graphics_queue_family_index, 0, std::addressof(m_vk_graphics_queue));

        /* Create graphics Command Pool */
        const VkCommandPoolCreateInfo graphics_command_pool_info = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .queueFamilyIndex = graphics_queue_family_index
        };
        
        result = ::vkCreateCommandPool(m_vk_device, std::addressof(graphics_command_pool_info), nullptr, std::addressof(m_vk_graphics_command_pool));
        DD_ASSERT(result == VK_SUCCESS);
        
        /* Query physical device memory properties */
        ::vkGetPhysicalDeviceMemoryProperties(m_vk_physical_device, std::addressof(m_vk_physical_device_memory_properties));

        m_graphics_queue_family_index = graphics_queue_family_index;
    }

    Context::~Context() {
        /* Finalize Vulkan context */
        ::vkDeviceWaitIdle(m_vk_device);
        
        ::vkDestroyCommandPool(m_vk_device, m_vk_graphics_command_pool, nullptr);
        ::vkDestroyDevice(m_vk_device, nullptr);
        #if defined(DD_DEBUG)
            ::vkDestroyDebugUtilsMessengerEXT(m_vk_instance, m_debug_messenger, nullptr);
        #endif
        ::vkDestroyInstance(m_vk_instance, nullptr);

        /* Free dynamic lists */
        if (m_vk_physical_device_array != nullptr) {
            delete[] m_vk_physical_device_array;
        }
    }
}
