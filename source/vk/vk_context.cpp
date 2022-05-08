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

namespace dd::vk {
    
    namespace {
        
        #if defined(DD_DEBUG)
            constexpr const char *DebugLayers[] = {
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
            VK_KHR_WIN32_SURFACE_EXTENSION_NAME
        };
        constexpr u32 InstanceExtensionCount = sizeof(InstanceExtensions) / sizeof(const char*);

        constexpr const char *DeviceExtensions[] = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
            VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME,
            VK_EXT_VERTEX_INPUT_DYNAMIC_STATE_EXTENSION_NAME,
            VK_EXT_EXTERNAL_MEMORY_HOST_EXTENSION_NAME
            
        };
        constexpr u32 DeviceExtensionCount = sizeof(DeviceExtensions) / sizeof(const char*);
        
        constinit VkPhysicalDeviceExtendedDynamicState2FeaturesEXT TargetDeviceExtendedDynamicStateFeatures = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_2_FEATURES_EXT,
            .extendedDynamicState2LogicOp = VK_TRUE
        };
        constinit VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT TargetDeviceVertexInputDynamicStateFeatures = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_INPUT_DYNAMIC_STATE_FEATURES_EXT,
            .pNext = std::addressof(TargetDeviceExtendedDynamicStateFeatures),
            .vertexInputDynamicState = VK_TRUE
        };
        constinit VkPhysicalDeviceVulkan13Features TargetDeviceFeatures13 = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
            .pNext = std::addressof(TargetDeviceVertexInputDynamicStateFeatures),
            .synchronization2 = VK_TRUE,
            .dynamicRendering = VK_TRUE
        };
        constinit VkPhysicalDeviceVulkan12Features TargetDeviceFeatures12 = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
            .pNext = std::addressof(TargetDeviceFeatures13),
            .descriptorIndexing = VK_TRUE,
            .shaderUniformBufferArrayNonUniformIndexing = VK_TRUE,
            .shaderSampledImageArrayNonUniformIndexing = VK_TRUE,
            .descriptorBindingSampledImageUpdateAfterBind = VK_TRUE,
            .descriptorBindingPartiallyBound = VK_TRUE,
            .descriptorBindingVariableDescriptorCount = VK_TRUE,
            .runtimeDescriptorArray = VK_TRUE,
            .bufferDeviceAddress = VK_TRUE
        };
        constinit VkPhysicalDeviceVulkan11Features TargetDeviceFeatures11 = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
            .pNext = std::addressof(TargetDeviceFeatures12),
            .variablePointersStorageBuffer = VK_TRUE,
            .variablePointers = VK_TRUE
        };
        constinit VkPhysicalDeviceFeatures2 TargetDeviceFeatures = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
            .pNext = std::addressof(TargetDeviceFeatures11),
            .features = {
                .independentBlend = VK_TRUE,
                .geometryShader = VK_TRUE,
                .tessellationShader = VK_TRUE,
                .logicOp = VK_TRUE,
                .depthClamp = VK_TRUE,
                .depthBiasClamp = VK_TRUE,
                .fillModeNonSolid = VK_TRUE,
                .depthBounds = VK_TRUE,
                .wideLines = VK_TRUE,
                .largePoints = VK_TRUE,
                .multiViewport = VK_TRUE,
                .samplerAnisotropy = VK_TRUE,
                .textureCompressionBC = VK_TRUE,
                .fragmentStoresAndAtomics = VK_TRUE,
                .shaderStorageImageExtendedFormats = VK_TRUE,
                .shaderUniformBufferArrayDynamicIndexing = VK_TRUE,
                .shaderSampledImageArrayDynamicIndexing = VK_TRUE,
                .shaderStorageBufferArrayDynamicIndexing = VK_TRUE,
                .shaderStorageImageArrayDynamicIndexing = VK_TRUE,
            }
        };
        
        static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity, VkDebugUtilsMessageTypeFlagsEXT message_type, const VkDebugUtilsMessengerCallbackDataEXT* callback_data, void* user_data) {
            /* Meme */
            DD_ASSERT(0xffff > message_severity);
            DD_ASSERT(message_type > 0);
            DD_ASSERT(user_data == nullptr);
            
            std::cout << "validation layer: " << callback_data->pMessage << std::endl;

            return VK_FALSE;
        }
        
        static LRESULT CALLBACK WndProc(HWND window_handle, UINT message, WPARAM w_param, LPARAM l_param) {
            if (message == WM_DESTROY) {
                ::PostQuitMessage(0);
                return 0;
            } else if (message == WM_PAINT) {
                return 0;
            } else if (message == WM_SIZE) {
                const u32 width = LOWORD(l_param);
                const u32 height = HIWORD(l_param);
                dd::vk::GetGlobalContext()->SetWindowDimensions(width, height);
            }

            return ::DefWindowProc(window_handle, message, w_param, l_param);
        }

            Context *global_context = nullptr;
    }
    
    void SetGlobalContext(Context *context) {
        global_context = context;
    }

    Context *GetGlobalContext() {
        return global_context;
    }

    bool Context::PickValidPhysicalDevice() {

        /* Hookup physical device properties chain*/
        m_vk_physical_device_properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
        m_vk_physical_device_properties.pNext = std::addressof(m_vk_physical_device_properties_11);
        m_vk_physical_device_properties_11.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_PROPERTIES;
        m_vk_physical_device_properties_11.pNext = std::addressof(m_vk_physical_device_properties_12);
        m_vk_physical_device_properties_12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_PROPERTIES;
        m_vk_physical_device_properties_12.pNext = std::addressof(m_vk_physical_device_properties_13);
        m_vk_physical_device_properties_13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_PROPERTIES;
        
        /* Hookup physical device features chain */
        m_vk_physical_device_supported_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        m_vk_physical_device_supported_features.pNext = std::addressof(m_vk_physical_device_supported_features_11);
        m_vk_physical_device_supported_features_11.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
        m_vk_physical_device_supported_features_11.pNext = std::addressof(m_vk_physical_device_supported_features_12);
        m_vk_physical_device_supported_features_12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
        m_vk_physical_device_supported_features_12.pNext = std::addressof(m_vk_physical_device_supported_features_13);
        m_vk_physical_device_supported_features_13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
        m_vk_physical_device_supported_features_13.pNext = std::addressof(m_vk_physical_device_vertex_input_dynamic_state_features);
        m_vk_physical_device_vertex_input_dynamic_state_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_INPUT_DYNAMIC_STATE_FEATURES_EXT;
        m_vk_physical_device_vertex_input_dynamic_state_features.pNext = std::addressof(m_vk_physical_device_extended_dynamic_state_features);
        m_vk_physical_device_extended_dynamic_state_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_2_FEATURES_EXT;
        
        for (u32 i = 0; i < m_vk_physical_device_count; ++i) {
            /* Query Physical Device */
            ::vkGetPhysicalDeviceProperties2(m_vk_physical_device_array[i], std::addressof(m_vk_physical_device_properties));
            ::vkGetPhysicalDeviceFeatures2(m_vk_physical_device_array[i], std::addressof(m_vk_physical_device_supported_features));

            /*  Ensure Vulkan 1.3 support */
            if (!(TargetMinimumApiVersion <= m_vk_physical_device_properties.properties.apiVersion)) { DD_ASSERT(false); continue; }
            
            /* Ensure resource ubos meet our size requirements */
            if (m_vk_physical_device_properties.properties.limits.maxUniformBufferRange < TargetMaxUniformBufferSize) { DD_ASSERT(false); continue; }

            /* Ensure present support */
            {
                m_vk_physical_device = m_vk_physical_device_array[i];
                u32 graphics_queue_family_index = 0;
                bool b_result1 = this->FindGraphicsQueueFamily(std::addressof(graphics_queue_family_index));
                DD_ASSERT(b_result1 == true);

                const u32 result = ::vkGetPhysicalDeviceWin32PresentationSupportKHR(m_vk_physical_device_array[i], graphics_queue_family_index);
                DD_ASSERT(result == VK_TRUE);

                m_vk_physical_device = 0;
            }

            /* Ensure support for targeted surface format */
            {
                u32 surface_format_count = 0;
                const u32 result0 = ::vkGetPhysicalDeviceSurfaceFormatsKHR(m_vk_physical_device_array[i], m_vk_surface, std::addressof(surface_format_count), nullptr);
                DD_ASSERT(result0 == VK_SUCCESS);

                VkSurfaceFormatKHR *surface_formats = new VkSurfaceFormatKHR[surface_format_count];
                const u32 result1 = ::vkGetPhysicalDeviceSurfaceFormatsKHR(m_vk_physical_device_array[i], m_vk_surface, std::addressof(surface_format_count), surface_formats);
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
                    DD_ASSERT(false); continue;
                }
            }

            /* Ensure support for targeted present mode */
            {
                u32 present_mode_count = 0;
                const u32 result0 = ::vkGetPhysicalDeviceSurfacePresentModesKHR(m_vk_physical_device_array[i], m_vk_surface, std::addressof(present_mode_count), nullptr);
                DD_ASSERT(result0 == VK_SUCCESS);

                VkPresentModeKHR *present_modes = new VkPresentModeKHR[present_mode_count];
                DD_ASSERT(present_modes != nullptr);
                
                const u32 result1 = ::vkGetPhysicalDeviceSurfacePresentModesKHR(m_vk_physical_device_array[i], m_vk_surface, std::addressof(present_mode_count), present_modes);
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
                    DD_ASSERT(false); continue;
                }
            }

            /* Ensure our memory alignment is compatible with out TargetMemoryAlignment */
            {
                const size_t vk_alignment = m_vk_physical_device_properties.properties.limits.minMemoryMapAlignment;
                if (TargetMemoryPoolAlignment % vk_alignment != 0)   { DD_ASSERT(false); continue; }
                if (!(TargetMemoryPoolAlignment / vk_alignment > 0)) { DD_ASSERT(false); continue; }
            }
            {
                const size_t vk_alignment = m_vk_physical_device_properties.properties.limits.minUniformBufferOffsetAlignment;
                if (TargetConstantBufferAlignment % vk_alignment != 0)   { DD_ASSERT(false); continue; }
                if (!(TargetConstantBufferAlignment / vk_alignment > 0)) { DD_ASSERT(false); continue; }
            }
            {
                const size_t vk_alignment = m_vk_physical_device_properties.properties.limits.minStorageBufferOffsetAlignment;
                if (TargetStorageBufferAlignment % vk_alignment != 0)   { DD_ASSERT(false); continue; }
                if (!(TargetStorageBufferAlignment / vk_alignment > 0)) { DD_ASSERT(false); continue; }
            }
            {
                const size_t vk_alignment = m_vk_physical_device_properties.properties.limits.minTexelBufferOffsetAlignment;
                if (TargetTexelBufferAlignment % vk_alignment != 0)   { DD_ASSERT(false); continue; }
                if (!(TargetTexelBufferAlignment / vk_alignment > 0)) { DD_ASSERT(false); continue; }
            }

            /* Ensure we can create the target amount of Descriptors */
            if (m_vk_physical_device_properties_12.maxUpdateAfterBindDescriptorsInAllPools       < TargetDescriptors)           { DD_ASSERT(false); continue; }
            if (m_vk_physical_device_properties_12.maxDescriptorSetUpdateAfterBindSampledImages  < TargetMaxTextureDescriptors) { DD_ASSERT(false); continue; }
            if (m_vk_physical_device_properties_12.maxDescriptorSetUpdateAfterBindSamplers       < TargetMaxSamplerDescriptors) { DD_ASSERT(false); continue; }
            if (m_vk_physical_device_properties.properties.limits.maxDescriptorSetUniformBuffers < TargetMaxBufferDescriptors)  { DD_ASSERT(false); continue; }

            /* Misc Feature checks */
            if (m_vk_physical_device_supported_features.features.independentBlend == false)                        { DD_ASSERT(false); continue; }
            if (m_vk_physical_device_supported_features.features.geometryShader == false)                          { DD_ASSERT(false); continue; }
            if (m_vk_physical_device_supported_features.features.tessellationShader == false)                      { DD_ASSERT(false); continue; }
            if (m_vk_physical_device_supported_features.features.logicOp == false)                                 { DD_ASSERT(false); continue; }
            if (m_vk_physical_device_supported_features.features.depthClamp == false)                              { DD_ASSERT(false); continue; }
            if (m_vk_physical_device_supported_features.features.depthBiasClamp == false)                          { DD_ASSERT(false); continue; }
            if (m_vk_physical_device_supported_features.features.fillModeNonSolid == false)                        { DD_ASSERT(false); continue; }
            if (m_vk_physical_device_supported_features.features.depthBounds == false)                             { DD_ASSERT(false); continue; }
            if (m_vk_physical_device_supported_features.features.wideLines == false)                               { DD_ASSERT(false); continue; }
            if (m_vk_physical_device_supported_features.features.largePoints == false)                             { DD_ASSERT(false); continue; }
            if (m_vk_physical_device_supported_features.features.multiViewport == false)                           { DD_ASSERT(false); continue; }
            if (m_vk_physical_device_supported_features.features.samplerAnisotropy == false)                       { DD_ASSERT(false); continue; }
            if (m_vk_physical_device_supported_features.features.textureCompressionBC == false)                    { DD_ASSERT(false); continue; }
            if (m_vk_physical_device_supported_features.features.fragmentStoresAndAtomics == false)                { DD_ASSERT(false); continue; }
            if (m_vk_physical_device_supported_features.features.shaderStorageImageExtendedFormats == false)       { DD_ASSERT(false); continue; }
            if (m_vk_physical_device_supported_features.features.shaderUniformBufferArrayDynamicIndexing == false) { DD_ASSERT(false); continue; }
            if (m_vk_physical_device_supported_features.features.shaderSampledImageArrayDynamicIndexing == false)  { DD_ASSERT(false); continue; }
            if (m_vk_physical_device_supported_features.features.shaderStorageBufferArrayDynamicIndexing == false) { DD_ASSERT(false); continue; }
            if (m_vk_physical_device_supported_features.features.shaderStorageImageArrayDynamicIndexing == false)  { DD_ASSERT(false); continue; }

            if (m_vk_physical_device_supported_features_11.variablePointersStorageBuffer == false)                 { DD_ASSERT(false); continue; }
            if (m_vk_physical_device_supported_features_11.variablePointers == false)                              { DD_ASSERT(false); continue; }

            if (m_vk_physical_device_supported_features_12.descriptorIndexing == false)                            { DD_ASSERT(false); continue; }
            if (m_vk_physical_device_supported_features_12.shaderUniformBufferArrayNonUniformIndexing == false)    { DD_ASSERT(false); continue; }
            if (m_vk_physical_device_supported_features_12.shaderSampledImageArrayNonUniformIndexing == false)     { DD_ASSERT(false); continue; }
            if (m_vk_physical_device_supported_features_12.descriptorBindingSampledImageUpdateAfterBind == false)  { DD_ASSERT(false); continue; }
            if (m_vk_physical_device_supported_features_12.descriptorBindingPartiallyBound == false)               { DD_ASSERT(false); continue; }
            if (m_vk_physical_device_supported_features_12.descriptorBindingVariableDescriptorCount == false)      { DD_ASSERT(false); continue; }
            if (m_vk_physical_device_supported_features_12.runtimeDescriptorArray == false)                        { DD_ASSERT(false); continue; }
            if (m_vk_physical_device_supported_features_12.bufferDeviceAddress == false)                           { DD_ASSERT(false); continue; }

            if (m_vk_physical_device_supported_features_13.synchronization2 == false)                              { DD_ASSERT(false); continue; }
            if (m_vk_physical_device_supported_features_13.dynamicRendering == false)                              { DD_ASSERT(false); continue; }

            if (m_vk_physical_device_vertex_input_dynamic_state_features.vertexInputDynamicState == false)         { DD_ASSERT(false); continue; }

            if (m_vk_physical_device_extended_dynamic_state_features.extendedDynamicState2LogicOp == false)        { DD_ASSERT(false); continue; }

            m_vk_physical_device = m_vk_physical_device_array[i];
            m_physical_device_index = i;
            return true;
        }

        return false;
    }

    bool Context::FindGraphicsQueueFamily(u32 *queue_family_index) {
        u32 queue_family_count = 0;
        ::vkGetPhysicalDeviceQueueFamilyProperties(m_vk_physical_device, std::addressof(queue_family_count), nullptr);

        VkQueueFamilyProperties *queue_properties = new VkQueueFamilyProperties[queue_family_count];
        ::vkGetPhysicalDeviceQueueFamilyProperties(m_vk_physical_device, std::addressof(queue_family_count), queue_properties);

        for (u32 i = 0; i < queue_family_count; ++i) {
            
            /* Every required graphics queue family feature should be checked here */
            if ((queue_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != VK_QUEUE_GRAPHICS_BIT) { continue; }
            
            {
                u32 supports_present = false;
                const u32 result0 = ::vkGetPhysicalDeviceSurfaceSupportKHR(m_vk_physical_device, i, m_vk_surface, std::addressof(supports_present));
                DD_ASSERT(result0 == VK_SUCCESS);
                
                if (supports_present == VK_FALSE) { continue; }
            }
            
            delete[] queue_properties;
            *queue_family_index = i;
            return true;
        }

        delete[] queue_properties;
        return false;
    }

    Context::Context() {
        
        dd::vk::SetGlobalContext(this);

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

        s32 result = ::vkCreateInstance(std::addressof(instance_info), nullptr, std::addressof(m_vk_instance));
        DD_ASSERT(result == VK_SUCCESS);
        
        /* Load instance procs */
        LoadCProcsInstance(m_vk_instance);

        #if defined(DD_DEBUG)
            /* Create debug messenger */
            const VkDebugUtilsMessengerCreateInfoEXT debug_messenger_info = {
                .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
                .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
                .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
                .pfnUserCallback = DebugCallback,
                .pUserData = nullptr
            };

            result = pfn_vkCreateDebugUtilsMessengerEXT(m_vk_instance, std::addressof(debug_messenger_info), nullptr, std::addressof(m_debug_messenger));
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

        m_hwnd = ::CreateWindow("VkDDWindow", "Window", WS_OVERLAPPEDWINDOW | WS_VISIBLE, 0, 0, 1280, 720, 0, 0, ::GetModuleHandle(nullptr), 0);
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
        DD_ASSERT(m_vk_physical_device_count != 0);

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

        m_graphics_queue_family_index = graphics_queue_family_index;

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

        /* Load device procs */
        LoadCProcsDevice(m_vk_device);

        /* Obtain graphics queue */
        ::vkGetDeviceQueue(m_vk_device, graphics_queue_family_index, 0, std::addressof(m_vk_graphics_queue));

        /* Create graphics Command Pool */
        const VkCommandPoolCreateInfo graphics_command_pool_info = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            .queueFamilyIndex = graphics_queue_family_index
        };
        
        result = ::vkCreateCommandPool(m_vk_device, std::addressof(graphics_command_pool_info), nullptr, std::addressof(m_vk_graphics_command_pool));
        DD_ASSERT(result == VK_SUCCESS);
        
        /* Query physical device memory properties */
        ::vkGetPhysicalDeviceMemoryProperties(m_vk_physical_device, std::addressof(m_vk_physical_device_memory_properties));
        
        /* Create resource buffer descriptor layout */
        const VkDescriptorSetLayoutBinding buffer_set_binding[] = { 
            {
                .binding         = Context::TargetVertexResourceBufferDescriptorBinding,
                .descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .descriptorCount = 1,
                .stageFlags      = VK_SHADER_STAGE_VERTEX_BIT,
            },
            {
                .binding         = Context::TargetTessellationEvaluationResourceBufferDescriptorBinding,
                .descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .descriptorCount = 1,
                .stageFlags      = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
            },
            {
                .binding         = Context::TargetTessellationControlResourceBufferDescriptorBinding,
                .descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .descriptorCount = 1,
                .stageFlags      = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
            },
            {
                .binding         = Context::TargetGeometryResourceBufferDescriptorBinding,
                .descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .descriptorCount = 1,
                .stageFlags      = VK_SHADER_STAGE_GEOMETRY_BIT,
            },
            {
                .binding         = Context::TargetFragmentResourceBufferDescriptorBinding,
                .descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .descriptorCount = 1,
                .stageFlags      = VK_SHADER_STAGE_FRAGMENT_BIT,
            },
            {
                .binding         = Context::TargetComputeResourceBufferDescriptorBinding,
                .descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .descriptorCount = 1,
                .stageFlags      = VK_SHADER_STAGE_COMPUTE_BIT,
            }
        };
        
        const VkDescriptorSetLayoutCreateInfo buffer_set_layout_info = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .bindingCount = sizeof(buffer_set_binding) / sizeof(VkDescriptorSetLayoutBinding),
            .pBindings = buffer_set_binding
        };
        const u32 result4 = ::vkCreateDescriptorSetLayout(m_vk_device, std::addressof(buffer_set_layout_info), nullptr, std::addressof(m_vk_resource_buffer_descriptor_set_layout));
        DD_ASSERT(result4 == VK_SUCCESS);

        /* Create texture descriptor layout */
        const VkDescriptorBindingFlags  texture_set_binding_flag[] = {
            VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT | VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT
        };
        const VkDescriptorSetLayoutBindingFlagsCreateInfo texture_set_binding_info = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
            .bindingCount = sizeof(texture_set_binding_flag) / sizeof(VkDescriptorBindingFlags),
            .pBindingFlags = texture_set_binding_flag
        };

        const VkDescriptorSetLayoutBinding texture_set_bindings[] = { 
            {
                .binding         = Context::TargetTextureDescriptorBinding,
                .descriptorType  = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                .descriptorCount = TargetMaxTextureDescriptors,
                .stageFlags      = VK_SHADER_STAGE_ALL,
            }
        };

        const VkDescriptorSetLayoutCreateInfo texture_set_layout_info = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .pNext = std::addressof(texture_set_binding_info),
            .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT,
            .bindingCount = sizeof(texture_set_bindings) / sizeof(VkDescriptorSetLayoutBinding),
            .pBindings = texture_set_bindings
        };
        const u32 result7 = ::vkCreateDescriptorSetLayout(m_vk_device, std::addressof(texture_set_layout_info), nullptr, std::addressof(m_vk_texture_descriptor_set_layout));
        DD_ASSERT(result7 == VK_SUCCESS);

        /* Create texture descriptor layout */
        const VkDescriptorBindingFlags  sampler_set_binding_flag[] = {
            VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT | VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT
        };
        const VkDescriptorSetLayoutBindingFlagsCreateInfo sampler_set_binding_info = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
            .bindingCount = sizeof(sampler_set_binding_flag) / sizeof(VkDescriptorBindingFlags),
            .pBindingFlags = sampler_set_binding_flag
        };

        const VkDescriptorSetLayoutBinding sampler_set_bindings[] = { 
            {
                .binding         = Context::TargetSamplerDescriptorBinding,
                .descriptorType  = VK_DESCRIPTOR_TYPE_SAMPLER,
                .descriptorCount = TargetMaxSamplerDescriptors,
                .stageFlags      = VK_SHADER_STAGE_ALL,
            }
        };

        const VkDescriptorSetLayoutCreateInfo sampler_set_layout_info = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .pNext = std::addressof(sampler_set_binding_info),
            .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT,
            .bindingCount = sizeof(sampler_set_bindings) / sizeof(VkDescriptorSetLayoutBinding),
            .pBindings = sampler_set_bindings
        };
        const u32 result6 = ::vkCreateDescriptorSetLayout(m_vk_device, std::addressof(sampler_set_layout_info), nullptr, std::addressof(m_vk_sampler_descriptor_set_layout));
        DD_ASSERT(result6 == VK_SUCCESS);

        /* Create global pipeline */
        const VkDescriptorSetLayout descriptor_set_layouts[] = {
            m_vk_texture_descriptor_set_layout,
            m_vk_sampler_descriptor_set_layout,
            m_vk_resource_buffer_descriptor_set_layout
        };
        
        const VkPushConstantRange resource_index_push_constant_ranges[] = {
            {
                .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
                .offset = 0,
                .size = sizeof(u32)
            },
            {
                .stageFlags = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
                .offset = sizeof(u32),
                .size = sizeof(u32)
            },
            {
                .stageFlags = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
                .offset = sizeof(u32) * 2,
                .size = sizeof(u32)
            },
            {
                .stageFlags = VK_SHADER_STAGE_GEOMETRY_BIT,
                .offset = sizeof(u32) * 3,
                .size = sizeof(u32)
            },
            {
                .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
                .offset = sizeof(u32) * 4,
                .size = sizeof(u32)
            },
            {
                .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
                .offset = sizeof(u32) * 5,
                .size = sizeof(u32)
            },
        };

        const VkPipelineLayoutCreateInfo pipeline_layout_info = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .setLayoutCount = sizeof(descriptor_set_layouts) / sizeof(VkDescriptorSetLayout),
            .pSetLayouts = descriptor_set_layouts,
            .pushConstantRangeCount = 6,
            .pPushConstantRanges = resource_index_push_constant_ranges
        };
        
        const u32 result9 = ::vkCreatePipelineLayout(m_vk_device, std::addressof(pipeline_layout_info), nullptr, std::addressof(m_vk_pipeline_layout));
        DD_ASSERT(result9 == VK_SUCCESS);
    }

    Context::~Context() {

        /* Finalize Vulkan context */
        ::vkQueueWaitIdle(m_vk_graphics_queue);
        ::vkDeviceWaitIdle(m_vk_device);

        ::vkDestroyPipelineLayout(m_vk_device, m_vk_pipeline_layout, nullptr);
        ::vkDestroyDescriptorSetLayout(m_vk_device, m_vk_resource_buffer_descriptor_set_layout, nullptr);
        ::vkDestroyDescriptorSetLayout(m_vk_device, m_vk_texture_descriptor_set_layout, nullptr);
        ::vkDestroyDescriptorSetLayout(m_vk_device, m_vk_sampler_descriptor_set_layout, nullptr);

        ::vkDestroyCommandPool(m_vk_device, m_vk_graphics_command_pool, nullptr);
        ::vkDestroyDevice(m_vk_device, nullptr);
        #if defined(DD_DEBUG)
            pfn_vkDestroyDebugUtilsMessengerEXT(m_vk_instance, m_debug_messenger, nullptr);
        #endif
        ::vkDestroySurfaceKHR(m_vk_instance, m_vk_surface, nullptr);
        ::vkDestroyInstance(m_vk_instance, nullptr);

        /* Free dynamic lists */
        if (m_vk_physical_device_array != nullptr) {
            delete[] m_vk_physical_device_array;
        }
    }
}
