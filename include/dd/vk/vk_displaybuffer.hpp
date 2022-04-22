#pragma once

namespace dd::vk {

    class DisplayBuffer {
        private:
            VkSwapchain m_vk_swapchain;
        public:
            Framebuffer() {/*...*/}

            void Initialize(const Context *context) {
                /* Create Swapchain */
                const u32 family_index = Context->GetGraphicsQueueFamilyIndex();
                const VkSwapchainCreateInfoKHR swapchain_info = {
                    .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO,
                    .surface = context->GetSurface(),
                    .minImageCount = 3,
                    .imageFormat = Context::TargetSurfaceFormat.format,
                    .imageColorSpace = Context::TargetSurfaceFormat.colorSpace,
                    .imageExtent = Context::GetWindowExtent(),
                    .imageArrayLayers = 1,
                    .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                    .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
                    .queueFamilyIndex = 1,
                    .pQueueFamilyIndices = std::addressof(family_index),
                    .presentMode = Context::TargetPresentMode
                };

                const u32 result = ::vkCreateSwapchainKHR(context->GetDevice(), std::addressof(swapchain_info), nullptr, std::addressof(m_vk_swapchain));
                DD_ASSERT(result != 0);
            }

            void Finalize(const Context *context) {
                ::vkDestroySwapchainKHR(Context->GetDevice(), m_vk_swapchain, nullptr);
            }

            void OnResize() {
                
            }
    };
}