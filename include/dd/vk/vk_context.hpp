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

    class CommandBuffer;
    class FrameBuffer;

    class Context {
        public:
            friend class FrameBuffer;
        public:
            /* Support Vulkan 1.3 */
            static constexpr u32 TargetMinimumApiVersion = VK_MAKE_API_VERSION(0, 1, 3, 0);

            /* Decide our memory pools must always be page aligned */
            static constexpr size_t TargetMemoryPoolAlignment = 0x1000;

            /* Decide our buffer limits */
            static constexpr size_t TargetConstantBufferAlignment    = 0x100;
            static constexpr size_t TargetStorageBufferAlignment     = 0x100;
            static constexpr size_t TargetTexelBufferAlignment       = 0x100;

            static constexpr size_t TargetMaxUniformBufferSize       = 0xFA00;

            /* Decide our global descriptor resource limits */
            static constexpr size_t TargetMaxTextureDescriptors      = 0x412c;
            static constexpr size_t TargetMaxSamplerDescriptors      = 0x1000;
            static constexpr size_t TargetMaxBufferDescriptors       = 6;
            static constexpr size_t TargetDescriptors                = TargetMaxTextureDescriptors + TargetMaxSamplerDescriptors + TargetMaxBufferDescriptors;

            /* Decide our global descriptor type bindings */
            static constexpr size_t TargetTextureDescriptorBinding                              = 0;
            static constexpr size_t TargetSamplerDescriptorBinding                              = 0;
            static constexpr size_t TargetVertexResourceBufferDescriptorBinding                 = 0;
            static constexpr size_t TargetTessellationEvaluationResourceBufferDescriptorBinding = 1;
            static constexpr size_t TargetTessellationControlResourceBufferDescriptorBinding    = 2;
            static constexpr size_t TargetGeometryResourceBufferDescriptorBinding               = 3;
            static constexpr size_t TargetFragmentResourceBufferDescriptorBinding               = 4;
            static constexpr size_t TargetComputeResourceBufferDescriptorBinding                = 5;

            /* Decide our per stage resource limits */
            static constexpr size_t TargetShaderStages = 6;

            static constexpr size_t TargetMaxPerStageUniformBufferCount = 14;
            static constexpr size_t TargetMaxPerStageStorageBufferCount = 16;
            static constexpr size_t TargetMaxPerStageTextureCount       = 32;
            static constexpr size_t TargetMaxPerStageStorageImageCount  = 8;

            /* Decide we are using a B8G8R8A8 Nonlinear SRGB format for our VkSurface */
            static constexpr VkSurfaceFormatKHR TargetSurfaceFormat = { 
                .format = VK_FORMAT_B8G8R8A8_SRGB,
                .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
            };

            /* Decide we will can have up to 8 color attachments per Render Pass */
            static constexpr size_t TargetColorAttachmentCount = 8;

            /* Decide we are using the Mailbox present mode */
            static constexpr VkPresentModeKHR TargetPresentMode = VK_PRESENT_MODE_FIFO_KHR;
        private:
            /* Vulkan objects */
            VkInstance                                          m_vk_instance;
            VkPhysicalDevice                                    m_vk_physical_device;
            VkDevice                                            m_vk_device;
            VkQueue                                             m_vk_graphics_queue;
            u32                                                 m_graphics_queue_family_index;
            VkCommandPool                                       m_vk_graphics_command_pool;
            VkSurfaceKHR                                        m_vk_surface;
            VkDescriptorSetLayout                               m_vk_resource_buffer_descriptor_set_layout;
            VkDescriptorSetLayout                               m_vk_texture_descriptor_set_layout;
            VkDescriptorSetLayout                               m_vk_sampler_descriptor_set_layout;
            VkPipelineLayout                                    m_vk_pipeline_layout;

            VkPhysicalDevice                                   *m_vk_physical_device_array;
            u32                                                 m_vk_physical_device_count;
            u32                                                 m_physical_device_index;

            VkPhysicalDeviceProperties2                         m_vk_physical_device_properties;
            VkPhysicalDeviceVulkan11Properties                  m_vk_physical_device_properties_11;
            VkPhysicalDeviceVulkan12Properties                  m_vk_physical_device_properties_12;
            VkPhysicalDeviceVulkan13Properties                  m_vk_physical_device_properties_13;

            VkPhysicalDeviceFeatures2                           m_vk_physical_device_supported_features;
            VkPhysicalDeviceVulkan11Features                    m_vk_physical_device_supported_features_11;
            VkPhysicalDeviceVulkan12Features                    m_vk_physical_device_supported_features_12;
            VkPhysicalDeviceVulkan13Features                    m_vk_physical_device_supported_features_13;
            VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT  m_vk_physical_device_vertex_input_dynamic_state_features;
            VkPhysicalDeviceExtendedDynamicState2FeaturesEXT    m_vk_physical_device_extended_dynamic_state_features;

            VkPhysicalDeviceMemoryProperties                    m_vk_physical_device_memory_properties;

            #if defined(DD_DEBUG)                               
                VkDebugUtilsMessengerEXT                        m_debug_messenger;
            #endif                                                                      

            /* Window objects */                                                        
            HWND                    m_hwnd;
            s32                     m_window_width;
            s32                     m_window_height;
            util::CriticalSection   m_window_cs;
            util::ConditionVariable m_window_cv;
            bool                    m_has_resized;

            /* Presentation objects */
            FrameBuffer                                                                *m_bound_frame_buffer;
            util::CriticalSection                                                       m_present_cs;
            util::TypeStorage<util::DelegateThread>                                     m_delegate_thread;
            util::TypeStorage<util::Delegate2<Context, util::DelegateThread*, size_t>>  m_present_delegate;
            bool                                                                        m_entered_present;
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

            constexpr ALWAYS_INLINE VkInstance GetInstance() const                                     { return m_vk_instance; }

            constexpr ALWAYS_INLINE VkSurfaceKHR GetSurface() const                                    { return m_vk_surface; }

            constexpr ALWAYS_INLINE VkPhysicalDevice GetPhysicalDevice() const                         { return m_vk_physical_device; }

            constexpr ALWAYS_INLINE VkDevice GetDevice() const                                         { return m_vk_device; }

            constexpr ALWAYS_INLINE VkQueue GetGraphicsQueue() const                                   { return m_vk_graphics_queue; }

            constexpr ALWAYS_INLINE u32 GetGraphicsQueueFamilyIndex() const                            { return m_graphics_queue_family_index; }

            constexpr ALWAYS_INLINE VkCommandPool GetGraphicsCommandPool() const                       { return m_vk_graphics_command_pool; }

            constexpr ALWAYS_INLINE VkDescriptorSetLayout GetTextureDescriptorSetLayout() const        { return m_vk_texture_descriptor_set_layout; }

            constexpr ALWAYS_INLINE VkDescriptorSetLayout GetSamplerDescriptorSetLayout() const        { return m_vk_sampler_descriptor_set_layout; }

            constexpr ALWAYS_INLINE VkDescriptorSetLayout GetResourceBufferDescriptorSetLayout() const { return m_vk_resource_buffer_descriptor_set_layout; }

            constexpr ALWAYS_INLINE VkPipelineLayout GetPipelineLayout() const                         { return m_vk_pipeline_layout; }

            constexpr ALWAYS_INLINE HWND GetWindowHandle() const                                       { return m_hwnd; }

            constexpr ALWAYS_INLINE const VkPhysicalDeviceProperties2 *GetPhysicalDeviceProperties() const { return std::addressof(m_vk_physical_device_properties); }
        
        public:

            void GetWindowDimensions(u32 *out_width, u32 *out_height) {
                std::scoped_lock l(m_window_cs);

                *out_width = m_window_width; 
                *out_height = m_window_height;
            }

            void BeginResize() {
                if (m_window_cs.IsLockedByCurrentThread() == false) {
                    m_window_cs.Enter();
                }
            }
            
            void EndResizeManuel() {
                m_window_cv.Broadcast();
                m_window_cs.Leave();
                m_has_resized = false;
            }

            void SetWindowDimensionsUnsafe(u32 width, u32 height) {
                m_window_width  = width;
                m_window_height = height;
                m_has_resized   = true;
            }
            
            void SetResize() {
                std::scoped_lock l(m_window_cs);
                m_has_resized   = true;
            }

            bool HasWindowResized() {
                std::scoped_lock l(m_window_cs);

                return m_has_resized == true;
            }

            void WaitSwapchainResize() {

                if (m_window_cs.IsLockedByCurrentThread() == true) {
                    while (m_has_resized == true) {
                        m_window_cv.Wait(std::addressof(m_window_cs));
                    }

                    m_window_cs.Leave();
                }
            }

            void FinishSwapchainResize() {
                m_has_resized = false;
                m_window_cv.Broadcast();
            }

        private:
            void PresentAsync(util::DelegateThread *thread, size_t message);
        public:
            void EnterDraw() { m_entered_present = false; }

            void Present(CommandBuffer *submit_command_buffer);

            void WaitForGpu();

            util::DelegateThread *InitializePresentationThread(FrameBuffer *framebuffer);
    };

    void SetGlobalContext(Context *context);

    Context *GetGlobalContext();
}
