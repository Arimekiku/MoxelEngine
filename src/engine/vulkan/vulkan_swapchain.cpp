#include "vulkan_swapchain.h"

#include <VkBootstrap.h>

namespace SDLarria 
{
	void VulkanSwapchain::Initialize(const VulkanInstance& toolset, const VkExtent2D& windowSize)
	{
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

		m_Images = vkbSwapchain.get_images().value();
		m_ImageViews = vkbSwapchain.get_image_views().value();
		m_SwapchainExtent = vkbSwapchain.extent;
	}

	void VulkanSwapchain::Destroy() 
	{
		vkDestroySwapchainKHR(m_DeviceInstance, m_SwapchainInstance, nullptr);

		for (const auto& view : m_ImageViews)
		{
			vkDestroyImageView(m_DeviceInstance, view, nullptr);
		}
	}
}
