#include "vulkan_swapchain.h"
#include "renderer/application.h"

#include <VkBootstrap.h>

namespace Moxel
{
	void VulkanSwapchain::initialize(const VkExtent2D& windowSize)
	{
		// build framebuffer
		m_framebuffer = std::make_shared<VulkanFramebuffer>();

		// build swapchain
		const auto& toolset = Application::get().get_context();
		m_deviceInstance = toolset.get_logical_device();

		const auto windowSurface = toolset.get_window_surface();
		const auto physicalDevice = toolset.get_physical_device();
		auto swapchainBuilder = vkb::SwapchainBuilder(physicalDevice, m_deviceInstance, windowSurface);

		auto swapchainFormat = VkSurfaceFormatKHR();
		swapchainFormat.format = VK_FORMAT_B8G8R8A8_SRGB;
		swapchainFormat.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
		m_swapchainImageFormat = swapchainFormat.format;

		vkb::Swapchain vkbSwapchain = swapchainBuilder
			.set_desired_format(swapchainFormat)
			.set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
			.set_desired_extent(windowSize.width, windowSize.height)
			.add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
			.build()
			.value();

		m_images = vkbSwapchain.get_images().value();
		m_imageViews = vkbSwapchain.get_image_views().value();
		m_swapchainInstance = vkbSwapchain.swapchain;
		m_swapchainExtent = vkbSwapchain.extent;
	}

	void VulkanSwapchain::resize()
	{
		const auto queue = Application::get().get_context().get_render_queue();
		auto& window = Application::get().get_window();

		vkQueueWaitIdle(queue);
		window.update_window_size();

		destroy();

		const auto windowSize = window.get_window_size();
		initialize(windowSize);
	}

	void VulkanSwapchain::destroy() 
	{
		m_framebuffer = nullptr;

		for (const auto& view : m_imageViews)
		{
			vkDestroyImageView(m_deviceInstance, view, nullptr);
		}

		vkDestroySwapchainKHR(m_deviceInstance, m_swapchainInstance, nullptr);
	}

	void VulkanSwapchain::update_frame(const CommandBufferData& reservedBuffer)
	{
		const auto device = Application::get().get_context().get_logical_device();

		const auto result = vkAcquireNextImageKHR(device, m_swapchainInstance, 1000000000, reservedBuffer.SwapchainSemaphore, nullptr, &m_currentFrameIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			resize();
		}

		m_currentFrame.ImageData = m_images[m_currentFrameIndex];
		m_currentFrame.ImageViewData = m_imageViews[m_currentFrameIndex];
	}

	void VulkanSwapchain::show_swapchain(const CommandBufferData& reservedBuffer)
	{
		const auto queue = Application::get().get_context().get_render_queue();

		// prepare present
		auto presentInfo = VkPresentInfoKHR();
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.pNext = nullptr;
		presentInfo.pSwapchains = &m_swapchainInstance;
		presentInfo.swapchainCount = 1;

		presentInfo.pWaitSemaphores = &reservedBuffer.RenderSemaphore;
		presentInfo.waitSemaphoreCount = 1;

		presentInfo.pImageIndices = &m_currentFrameIndex;

		const auto result = vkQueuePresentKHR(queue, &presentInfo);
		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			resize();
		}
	}
}
