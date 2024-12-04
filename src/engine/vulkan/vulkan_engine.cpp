#include "vulkan_engine.h"
#include "vulkan_image.h"

#include <cmath>
//#include <VkBootstrap.h>
#include <SDL3/SDL_vulkan.h>

namespace SDLarria {
	void VulkanEngine::Initialize(SDL_Window* window, VkExtent2D windowSize) {
         auto builder = vkb::InstanceBuilder();
         auto vkbInstance = builder
             .set_app_name("Vulkan Engine Application")
             .request_validation_layers(true)
             .use_default_debug_messenger()
             .require_api_version(1, 3, 0)
             .build()
             .value();

         m_Instance = vkbInstance.instance;
         m_DebugUtils = vkbInstance.debug_messenger;

         SDL_Vulkan_CreateSurface(window, m_Instance, nullptr, &m_WindowSurface);

         //vulkan 1.3 features
         VkPhysicalDeviceVulkan13Features features{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES };
         features.dynamicRendering = true;
         features.synchronization2 = true;

         //vulkan 1.2 features
         VkPhysicalDeviceVulkan12Features features12{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES };
         features12.bufferDeviceAddress = true;
         features12.descriptorIndexing = true;

         auto selector = vkb::PhysicalDeviceSelector(vkbInstance);
         auto physicalDevice = selector
             .set_minimum_version(1, 3)
             .set_required_features_13(features)
             .set_required_features_12(features12)
             .set_surface(m_WindowSurface)
             .select()
             .value();

         auto deviceBuilder = vkb::DeviceBuilder(physicalDevice);
         auto vkbDevice = deviceBuilder.build().value();

         m_LogicalDevice = vkbDevice.device;
         m_PhysicalDevice = physicalDevice.physical_device;

         m_Swapchain.Initialize(vkbDevice, m_WindowSurface, windowSize);
         m_CommandPool.Initialize(vkbDevice);
	}

	 void VulkanEngine::Draw() {
         // TODO: log this
         // Update fences
         vkWaitForFences(m_LogicalDevice, 1, &m_CommandPool.GetLastFrame().RenderFence, true, 1000000000);
         vkResetFences(m_LogicalDevice, 1, &m_CommandPool.GetLastFrame().RenderFence);

         uint32_t swapchainImageIndex;
         const auto& lastFrame = m_CommandPool.GetLastFrame();
         vkAcquireNextImageKHR(m_LogicalDevice, m_Swapchain.m_SwapchainInstance, 1000000000, lastFrame.SwapchainSemaphore, nullptr, &swapchainImageIndex);

         VkCommandBuffer cmd = lastFrame.CommandBuffer;

         // Render commands
         vkResetCommandBuffer(cmd, 0);

         auto cmdBeginInfo = VkCommandBufferBeginInfo();
         cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
         cmdBeginInfo.pNext = nullptr;
         cmdBeginInfo.pInheritanceInfo = nullptr;
         cmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

         vkBeginCommandBuffer(cmd, &cmdBeginInfo);

         // make the swapchain image into writeable mode before rendering
         auto& image = m_Swapchain.m_Images[swapchainImageIndex];
         VulkanImage::WriteImage(cmd, image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

         // make a clear-color from frame number. This will flash with a 120 frame period.
         VkClearColorValue clearValue;
         float flash = std::abs(std::sin(120 / 120.f));
         clearValue = { { 0.0f, 0.0f, flash, 1.0f } };

         auto clearRange = VkImageSubresourceRange();
         clearRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
         clearRange.baseMipLevel = 0;
         clearRange.levelCount = VK_REMAINING_MIP_LEVELS;
         clearRange.baseArrayLayer = 0;
         clearRange.layerCount = VK_REMAINING_ARRAY_LAYERS;

         // clear image
         vkCmdClearColorImage(cmd, image, VK_IMAGE_LAYOUT_GENERAL, &clearValue, 1, &clearRange);

         // make the swapchain image into presentable mode
         VulkanImage::WriteImage(cmd, image, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

         // finalize the command buffer (we can no longer add commands, but it can now be executed)
         vkEndCommandBuffer(cmd);

         // prepare the submission to the queue.
         // we want to wait on the _presentSemaphore, as that semaphore is signaled when the swapchain is ready
         // we will signal the _renderSemaphore, to signal that rendering has finished
         VkCommandBufferSubmitInfo cmdinfo = VkCommandBufferSubmitInfo();
         cmdinfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
         cmdinfo.pNext = nullptr;
         cmdinfo.commandBuffer = cmd;
         cmdinfo.deviceMask = 0;

         auto waitInfo = VkSemaphoreSubmitInfo();
         waitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
         waitInfo.pNext = nullptr;
         waitInfo.semaphore = lastFrame.SwapchainSemaphore;
         waitInfo.stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR;
         waitInfo.deviceIndex = 0;
         waitInfo.value = 1;

         auto signalInfo = VkSemaphoreSubmitInfo();
         signalInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
         signalInfo.pNext = nullptr;
         signalInfo.semaphore = lastFrame.SwapchainSemaphore;
         signalInfo.stageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT;
         signalInfo.deviceIndex = 0;
         signalInfo.value = 1;

         auto submit = VkSubmitInfo2();
         submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
         submit.pNext = nullptr;
         submit.waitSemaphoreInfoCount = 1;
         submit.pWaitSemaphoreInfos = &waitInfo;
         submit.signalSemaphoreInfoCount = 1;
         submit.pSignalSemaphoreInfos = &signalInfo;
         submit.commandBufferInfoCount = 1;
         submit.pCommandBufferInfos = &cmdinfo;

         // submit command buffer to the queue and execute it.
         // Fence will now block until the graphic commands finish execution
         vkQueueSubmit2(m_CommandPool.m_GraphicsQueue, 1, &submit, lastFrame.RenderFence);

         // prepare present
         // this will put the image we just rendered to into the visible window.
         // we want to wait on the _renderSemaphore for that,
         // as its necessary that drawing commands have finished before the image is displayed to the user
         auto presentInfo = VkPresentInfoKHR();
         presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
         presentInfo.pNext = nullptr;
         presentInfo.pSwapchains = &m_Swapchain.m_SwapchainInstance;
         presentInfo.swapchainCount = 1;

         presentInfo.pWaitSemaphores = &lastFrame.RenderSemaphore;
         presentInfo.waitSemaphoreCount = 1;

         presentInfo.pImageIndices = &swapchainImageIndex;

         vkQueuePresentKHR(m_CommandPool.m_GraphicsQueue, &presentInfo);

         //increase the number of frames drawn
         m_CommandPool.m_CurrentFrame++;
	 }

	void VulkanEngine::Shutdown() {
        m_Swapchain.Destroy();

        vkDestroySurfaceKHR(m_Instance, m_WindowSurface, nullptr);
        vkDestroyDevice(m_LogicalDevice, nullptr);

        vkb::destroy_debug_utils_messenger(m_Instance, m_DebugUtils);
        vkDestroyInstance(m_Instance, nullptr);
	}
}