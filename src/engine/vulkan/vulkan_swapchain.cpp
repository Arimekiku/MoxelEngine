#include "vulkan_swapchain.h"
#include "vulkan.h"

#include <VkBootstrap.h>

namespace SDLarria 
{
	void VulkanSwapchain::Initialize(const VulkanInstance& toolset, const VkExtent2D& windowSize)
	{
		// build swapchain
		m_DeviceInstance = toolset.GetLogicalDevice();

		VkSurfaceKHR windowSurface = toolset.GetWindowSurface();
		VkPhysicalDevice physicalDevice = toolset.GetPhysicalDevice();
		auto swapchainBuilder = vkb::SwapchainBuilder(physicalDevice, m_DeviceInstance, windowSurface);

		auto swapchainFormat = VkSurfaceFormatKHR();
		swapchainFormat.format = VK_FORMAT_B8G8R8A8_UNORM;
		swapchainFormat.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

		vkb::Swapchain vkbSwapchain = swapchainBuilder
			.set_desired_format(swapchainFormat)
			.set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
			.set_desired_extent(windowSize.width, windowSize.height)
			.add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
			.build()
			.value();

		m_SwapchainInstance = vkbSwapchain.swapchain;
		m_SwapchainImageFormat = swapchainFormat.format;

		// create frames
		m_Frames.resize(vkbSwapchain.image_count);
		for (int i = 0; i < vkbSwapchain.image_count; i++)
		{
			auto& frame = m_Frames[i];

			frame.ImageData = vkbSwapchain.get_images().value()[i];
			frame.ImageViewData = vkbSwapchain.get_image_views().value()[i];
		}

		m_SwapchainExtent = vkbSwapchain.extent;
	}

	void VulkanSwapchain::Destroy() 
	{
		vkDestroySwapchainKHR(m_DeviceInstance, m_SwapchainInstance, nullptr);

		for (const auto& frame : m_Frames)
		{
			vkDestroyImageView(m_DeviceInstance, frame.ImageViewData, nullptr);
		}
	}

	FrameData& VulkanSwapchain::GetCurrentFrame(const CommandBufferData& reservedBuffer)
	{
		auto device = VulkanRenderer::Get().GetInstance().GetLogicalDevice();

		auto result = vkAcquireNextImageKHR(device, m_SwapchainInstance, 1000000000, reservedBuffer.SwapchainSemaphore, nullptr, &m_CurrentFrameIndex);
		VULKAN_CHECK(result);

		m_CurrentFrame = m_Frames[m_CurrentFrameIndex];
		return m_CurrentFrame;
	}

	void VulkanSwapchain::ShowSwapchain(const CommandBufferData& reservedBuffer) const
	{
		auto queue = VulkanRenderer::Get().GetInstance().GetRenderQueue();

		// prepare present
		auto presentInfo = VkPresentInfoKHR();
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.pNext = nullptr;
		presentInfo.pSwapchains = &m_SwapchainInstance;
		presentInfo.swapchainCount = 1;

		presentInfo.pWaitSemaphores = &reservedBuffer.RenderSemaphore;
		presentInfo.waitSemaphoreCount = 1;

		presentInfo.pImageIndices = &m_CurrentFrameIndex;

		auto result = vkQueuePresentKHR(queue, &presentInfo);
		VULKAN_CHECK(result);
	}
}
