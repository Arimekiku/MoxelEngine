#include "vulkan_engine.h"
#include "vulkan_image.h"
#include "vulkan.h"

#include <cmath>

namespace SDLarria 
{
	void VulkanEngine::Initialize(SDL_Window* window, VkExtent2D windowSize) 
    {
         m_Instance.Initialize(window);

         m_Swapchain.Initialize(m_Instance, windowSize);
         m_CommandPool.Initialize(m_Instance);
	}

	 void VulkanEngine::Draw() 
     {
         const auto& logicalDevice = m_Instance.GetLogicalDevice();

         // Update fences
         auto result = vkWaitForFences(logicalDevice, 1, &m_CommandPool.GetLastFrame().RenderFence, true, 1000000000);
         VULKAN_CHECK(result);

         result = vkResetFences(logicalDevice, 1, &m_CommandPool.GetLastFrame().RenderFence);
         VULKAN_CHECK(result);

         uint32_t swapchainImageIndex;
         const auto& lastFrame = m_CommandPool.GetLastFrame();
         result = vkAcquireNextImageKHR(logicalDevice, m_Swapchain.m_SwapchainInstance, 1000000000, lastFrame.SwapchainSemaphore, nullptr, &swapchainImageIndex);
         VULKAN_CHECK(result);

         VkCommandBuffer cmd = lastFrame.CommandBuffer;

         // Render commands
         result = vkResetCommandBuffer(cmd, 0);
         VULKAN_CHECK(result);

         auto cmdBeginInfo = VkCommandBufferBeginInfo();
         cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
         cmdBeginInfo.pNext = nullptr;
         cmdBeginInfo.pInheritanceInfo = nullptr;
         cmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

         result = vkBeginCommandBuffer(cmd, &cmdBeginInfo);
         VULKAN_CHECK(result);

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
         result = vkEndCommandBuffer(cmd);
         VULKAN_CHECK(result);

         // prepare the submission to the queue.
         // we want to wait on the _presentSemaphore, as that semaphore is signaled when the swapchain is ready
         // we will signal the _renderSemaphore, to signal that rendering has finished
         auto cmdInfo = VkCommandBufferSubmitInfo();
         cmdInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
         cmdInfo.pNext = nullptr;
         cmdInfo.commandBuffer = cmd;
         cmdInfo.deviceMask = 0;

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
         signalInfo.semaphore = lastFrame.RenderSemaphore;
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
         submit.pCommandBufferInfos = &cmdInfo;

         // submit command buffer to the queue and execute it.
         // Fence will now block until the graphic commands finish execution
         result = vkQueueSubmit2(m_CommandPool.m_GraphicsQueue, 1, &submit, lastFrame.RenderFence);
         VULKAN_CHECK(result);

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

         result = vkQueuePresentKHR(m_CommandPool.m_GraphicsQueue, &presentInfo);
         VULKAN_CHECK(result);

         //increase the number of frames drawn
         m_CommandPool.m_CurrentFrame++;
	 }

	void VulkanEngine::Shutdown() 
    {
        m_Swapchain.Destroy();
        m_CommandPool.Destroy();
        m_Instance.Destroy();
	}
}