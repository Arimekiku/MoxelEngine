#include "vulkan_swapchain.h"

namespace SDLarria {
	void VulkanSwapchain::Initialize(vkb::Device& device, VkSurfaceKHR surface, VkExtent2D& surfaceExtent) {
		auto& physicalDevice = device.physical_device;
		m_DeviceInstance = device.device;

		auto swapchainBuilder = vkb::SwapchainBuilder(physicalDevice, m_DeviceInstance, surface);

		m_SwapchainImageFormat = VK_FORMAT_B8G8R8A8_UNORM;

		vkb::Swapchain vkbSwapchain = swapchainBuilder
			.set_desired_format(VkSurfaceFormatKHR{ .format = m_SwapchainImageFormat, .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR })
			.set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
			.set_desired_extent(surfaceExtent.width, surfaceExtent.height)
			.add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
			.build()
			.value();

		m_SwapchainInstance = vkbSwapchain.swapchain;

		m_Images = vkbSwapchain.get_images().value();
		m_ImageViews = vkbSwapchain.get_image_views().value();
		m_SwapchainExtent = vkbSwapchain.extent;
	}

	void VulkanSwapchain::Destroy() {
		vkDestroySwapchainKHR(m_DeviceInstance, m_SwapchainInstance, nullptr);

		for (auto& view : m_ImageViews) {
			vkDestroyImageView(m_DeviceInstance, view, nullptr);
		}
	}
}