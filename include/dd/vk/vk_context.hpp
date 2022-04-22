#pragma once

namespace dd::vk {

    namespace {
        #if defined(DD_DEBUG)
            constexpr const char *DebugLayers = {
                "VK_LAYER_KHRONOS_validation"
            };
            constexpr u32 DebugLayerCount = sizeof(DebugLayers) / sizeof(const char*);
        #endif

        constexpr const char *DeviceExtensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };
        constexpr u32 DeviceExtensionCount = sizeof(DeviceExtensions) / sizeof(const char*);
    }

    class Context {
        public:
            constexpr u32 TargetMinimumApiVersion = VK_MAKE_API_VERSION(1, 3, 0, 0);
            constexpr VkSurfaceFormatKHR TargetSurfaceFormat = { .format = VK_FORMAT_B8G8R8A8_SRGB, .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR };
            constexpr VkPresentModeKHR TargetPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
            
        private:
            VkInstance        m_vk_instance;
            VkPhysicalDevice  m_vk_physical_device;
            VkDevice          m_vk_device;
            VkQueue           m_vk_graphics_queue;
            u32               m_graphics_queue_family_index;
            VkCommandPool     m_vk_graphics_command_pool;
            VkSurface         m_vk_surface;
            
            HWND              m_hwnd;
            
            VkPhysicalDevice *m_vk_physical_device_array;
            u32               m_vk_physical_device_count;
            
            VkPhysicalDeviceProperties2 *m_vk_physical_device_properties_array;
            
            VkPhysicalDeviceMemoryProperties m_vk_physical_device_memory_properties;

            #if defined(DD_DEBUG)
                VkDebugUtilsMessengerEXT m_debug_messenger;
            #endif
        private:
            #if defined(DD_DEBUG)
                static VKAPI_ATTR Vkbool32 VKAPI_CALL DebugCallback(VkDebugUtilMessageSeverityFlagBitsExt message_severity, VkDebugUtilsMessageTypeFlagsEXT message_type, const VkDebugUtilsMessengerCallbackDataEXT *callback_data, void *user_data) {
                    std::cout << "validation layer: " << callback_data->pMessage << std::endl;

                    return VK_FALSE;
                }
            #endif

            VkPhysicalDevice PickValidPhysicalDevice() {
                for (u32 i = 0; i < m_vk_physical_device_count; ++i) {
                    
                    /* Ensure present support */
                    u32 result = ::vkGetPhysicalDeviceWin32PresentationSupportKHR(m_vk_physical_device, graphics_queue_family_index);
                    DD_ASSERT(result == VK_SUCCESS);
                    
                    /* Ensure support for targeted surface format */
                    u32 surface_format_count = 0;
                    result = ::vkGetPhysicalDeviceSurfaceFormatsKHR(m_vk_device, m_vk_surface, std::addressof(surface_format_count), nullptr);
                    DD_ASSERT(result == VK_SUCCESS);

                    VkSurfaceFormatKHR *surface_formats = new VkSurfaceFormatKHR[surface_format_count];
                    result = ::vkGetPhysicalDeviceSurfaceFormatsKHR(m_vk_device, m_vk_surface, std::addressof(surface_format_count), std::addessof(surface_formats));
                    DD_ASSERT(result == VK_SUCCESS);
                    
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
                    
                    /* Ensure support for targeted present mode */
                    u32 present_mode_count = 0;
                    result = ::vkGetPhysicalDeviceSurfacePresentModesKHR(m_vk_physical_device, m_vk_surface, std::addressof(present_mode_count), nullptr);
                    DD_ASSERT(result == VK_SUCCESS);
                    
                    VkSurfaceFormatKHR present_modes = new VkSurfaceFormatKHR[present_mode_count];
                    result = ::vkGetPhysicalDeviceSurfacePresentModesKHR(m_vk_physical_device, m_vk_surface, std::addressof(present_mode_count), std::addressof(present_modes));
                    DD_ASSERT(result == VK_SUCCESS);
                    
                    for (u32 i = 0; i < surface_format_count; ++i) {
                        if (present_modes[i].format == TargetPresentMode) {
                            delete [] present_modes;
                            present_modes = nullptr;
                            break;
                        }
                    }
                    if (present_modes != nullptr) {
                        delete [] present_modes;
                        continue;
                    }

                    /* Every requires application feature should be checked here */
                    if (!(TargetMinimumApiVersion <= m_vk_physical_device_properties_array[i].properties.apiVersion)) { continue; }

                    return m_vk_physical_device_array[i];
                }

                return nullptr;
            }

            bool FindGraphicsQueueFamily(u32 *queue_family_index) {
                u32 queue_family_count = 0;
                ::vkGetPhysicalDeviceQueueFamilyProperties(m_vk_physical_device, std::addressof(queue_family_count), nullptr);

                VkQueueFamilyProperties queue_properties = new VkQueueFamilyProperties[queue_family_count];
                ::vkGetPhysicalDeviceQueueFamilyProperties(m_vk_physical_device, std::addressof(queue_family_count), std::addressof(queue_properties));

                for (u32 i = 0; i < queue_family_count; ++i) {
                    
                    /* Every required graphics queue family feature should be checked here */
                    if (!(queue_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT != VK_QUEUE_GRAPHICS_BIT)) { continue; }
                    
                    delete[] queue_properties;
                    *queue_family_index = i;
                    return true;
                }

                delete[] queue_properties;
                return false;
            }

            static LRESULT CALLBACK WndProc(HWND window_handle, UINT message, WPARAM w_param, LPARAM l_param) {
                if (message == WM_DESTROY) {
                    ::PostQuitMessage(0);
                    return 0;
                } else if (message == WM_PAINT) {
                    return 0;
                } else if (message == WM_SIZE) {
                    dd::util::SetFrameFrequency(::GetDC(window_handle));
                    (pfn_glViewport)(0, 0, LOWORD(l_param), HIWORD(l_param));
                } else if (message == WM_DISPLAYCHANGE || message == WM_MOVE) {
                    dd::util::SetFrameFrequency(::GetDC(window_handle));
                } else if (message == WM_INPUT) {
                    dd::hid::SetLastRawInput(reinterpret_cast<HRAWINPUT>(l_param));
                }

                return ::DefWindowProc(window_handle, message, w_param, l_param);
            }
        public:
            explicit DeviceContext() {
                /* Check supported Vulkan Api version */
                u32 api_version = 0;
                ::vkEnumerateInstanceVersion(std::addressof(api_version));
                DD_ASSERT(TargetMinimumApiVersion <= api_version);

                /* Create instance */
                const VkApplicationInfo app_info = {
                    .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
                    .pApplicationName = u8"Learn";
                    .applicationVersion = 1
                    .pEngineName = u8"DD";
                    .engineVersion = 1;
                    .apiVersion = TargetMinimumApiVersion
                }

                const VkInstanceCreateInfo instance_info = {
                    .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
                    .pApplicationInfo = std::addressof(app_info),
                    #if defined(DD_DEBUG)
                        .enabledLayerCount = DebugLayerCount,
                        .ppEnabledLayerNames = DebugLayers,
                    #endif
                }

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
                u32 result = ::RegisterClass(std::addressof(wc));
                DD_ASSERT(result != 0);

                m_hwnd = ::CreateWindow("VkDDWindow", "Window", WS_OVERLAPPEDWINDOW, 0, 0, 1280, 720, 0, 0, ::GetModuleHandle(nullptr), 0);
                DD_ASSERT(m_hwnd != nullptr);

                /* Create Surface */
                const VkWin32SurfaceCreateInfoKHR win32_info {
                    .sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO,
                    .hInstance = process_handle;
                    .hwnd = hwnd
                };

                result = ::vkCreateWin32SurfaceKHR(context->GetInstance(), std::addressof(win32_info), nullptr, std::addressof(m_vk_surface));
                DD_ASSERT(result == VK_SUCCESS);

                /* Enumerate Physical Devices into list */
                result = ::vkEnumeratePhysicalDevices(m_vk_instance, std::addressof(m_vk_physical_device_count), nullptr);
                DD_ASSERT(result == VK_SUCCESS);

                m_vk_physical_device_array = new VkPhysicalDevice[m_vk_physical_device_count];
                DD_ASSERT(m_vk_physical_device_array != nullptr);

                result = ::vkEnumeratePhysicalDevices(m_vk_instance, std::addressof(m_vk_physical_device_count), std::addressof(m_vk_physical_device_array));
                DD_ASSERT(result == VK_SUCCESS);

                /* Acquire our Physical Device properties */
                m_vk_physical_device_properties_array = new VkPhysicalDeviceProperties2[m_vk_physical_device_count];
                DD_ASSERT(m_vk_physical_device_properties_array != nullptr);

                for (u32 i = 0; i < m_vk_physical_device_count; ++i) {
                    m_vk_physical_device_properties_array[i].sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
                    ::vkGetPhysicalDeviceProperties2(m_vk_physical_device_array[i], std::addressof(m_vk_physical_device_properties_array[i]));
                }

                /* Pick our first valid Physical Device */
                m_vk_physical_device = this->PickValidPhysicalDevice();
                DD_ASSERT(m_vk_physical_device != nullptr);

                /* Obtain a valid graphics queue family */
                u32 graphics_queue_family_index = 0;
                bool b_result = this->FindGraphicsQueueFamily(std::addressof(graphics_queue_family_index));
                DD_ASSERT(b_result == true);

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

            ~DeviceContext() {
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

                if (m_vk_physical_device_properties_array != nullptr) {
                    delete[] m_vk_physical_device_properties_array;
                }
            }
            
            constexpr ALWAYS_INLINE s32 FindMemoryHeapIndex(u32 memory_properties) const {
                for (s32 i = 0; i < m_vk_physical_device_memory_properties.memoryTypeCount; ++i) {
                    if ((m_vk_physical_device_memory_properties.memoryTypes[i].propertyFlags & memory_properties) == memory_properties) {
                        return i;
                    }
                }

                return -1;
            }

            constexpr ALWAYS_INLINE VkInstance GetInstance() const               { return m_vk_instance; }

            constexpr ALWAYS_INLINE VkSurface GetSurface() const                 { return m_vk_surface; }

            constexpr ALWAYS_INLINE VkPhysicalDevice GetPhysicalDevice() const   { return m_vk_physical_device; }

            constexpr ALWAYS_INLINE VkDevice GetDevice() const                   { return m_vk_device; }

            constexpr ALWAYS_INLINE VkQueue GetGraphicsQueue() const             { return m_vk_graphics_queue; }

            constexpr ALWAYS_INLINE VkCommandPool GetGraphicsCommandPool() const { return m_vk_graphics_command_pool; }
    };
}