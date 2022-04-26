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

namespace dd::vk {

    class Context {
        public:
            /* Support Vulkan 1.3 */
            static constexpr u32 TargetMinimumApiVersion = VK_MAKE_API_VERSION(1, 3, 0, 0);
            /* Decide our memory pools must be page aligned */
            static constexpr size_t TargetMemoryPoolAlignment = 0x1000;
            /* Decide we are using a B8G8R8A8 Nonlinear SRGB format for our VkSurface */
            static constexpr VkSurfaceFormatKHR TargetSurfaceFormat = { .format = VK_FORMAT_B8G8R8A8_SRGB, .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
            /* Decide we are using the Mailbox present mode */
            static constexpr VkPresentModeKHR TargetPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
        private:
            VkInstance        m_vk_instance;
            VkPhysicalDevice  m_vk_physical_device;
            VkDevice          m_vk_device;
            VkQueue           m_vk_graphics_queue;
            u32               m_graphics_queue_family_index;
            VkCommandPool     m_vk_graphics_command_pool;
            VkSurfaceKHR      m_vk_surface;

            HWND              m_hwnd;

            VkPhysicalDevice *m_vk_physical_device_array;
            u32               m_vk_physical_device_count;
            u32               m_physical_device_index;

            VkPhysicalDeviceProperties2      m_vk_physical_device_properties;
            VkPhysicalDeviceFeatures2        m_vk_physical_device_supported_features;
            VkPhysicalDeviceVulkan11Features m_vk_physical_device_supported_features_11;
            VkPhysicalDeviceVulkan12Features m_vk_physical_device_supported_features_12;
            VkPhysicalDeviceVulkan13Features m_vk_physical_device_supported_features_13;
            VkPhysicalDeviceMemoryProperties m_vk_physical_device_memory_properties;

            #if defined(DD_DEBUG)
                VkDebugUtilsMessengerEXT m_debug_messenger;
            #endif
        private:

            bool PickValidPhysicalDevice();

            bool FindGraphicsQueueFamily(u32 *queue_family_index);
        public:
            explicit Context();

            ~Context();
            
            constexpr ALWAYS_INLINE s32 FindMemoryHeapIndex(u32 memory_properties) const {
                for (s32 i = 0; i < m_vk_physical_device_memory_properties.memoryTypeCount; ++i) {
                    if ((m_vk_physical_device_memory_properties.memoryTypes[i].propertyFlags & memory_properties) == memory_properties) {
                        return i;
                    }
                }

                return -1;
            }

            constexpr ALWAYS_INLINE VkInstance GetInstance() const               { return m_vk_instance; }

            constexpr ALWAYS_INLINE VkSurfaceKHR GetSurface() const              { return m_vk_surface; }

            constexpr ALWAYS_INLINE VkPhysicalDevice GetPhysicalDevice() const   { return m_vk_physical_device; }

            constexpr ALWAYS_INLINE VkDevice GetDevice() const                   { return m_vk_device; }

            constexpr ALWAYS_INLINE VkQueue GetGraphicsQueue() const             { return m_vk_graphics_queue; }

            constexpr ALWAYS_INLINE u32 GetGraphicsQueueFamilyIndex() const      { return m_graphics_queue_family_index; }

            constexpr ALWAYS_INLINE VkCommandPool GetGraphicsCommandPool() const { return m_vk_graphics_command_pool; }
    };
}