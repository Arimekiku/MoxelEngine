#include "vulkan_context.h"

#include <VkBootstrap.h>
#include <SDL3/SDL_vulkan.h>

namespace Moxel
{
	void VulkanContext::Initialize(SDL_Window* window) 
	{
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

		// vulkan 1.3 features
		auto features13 = VkPhysicalDeviceVulkan13Features();
		features13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
		features13.dynamicRendering = true;
		features13.synchronization2 = true;

		// vulkan 1.2 features
		auto features12 = VkPhysicalDeviceVulkan12Features();
		features12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
		features12.bufferDeviceAddress = true;
		features12.descriptorIndexing = true;

		auto selector = vkb::PhysicalDeviceSelector(vkbInstance);
		auto physicalDevice = selector
			.set_minimum_version(1, 3)
			.set_required_features_13(features13)
			.set_required_features_12(features12)
			.set_surface(m_WindowSurface)
			.select()
			.value();

		auto deviceBuilder = vkb::DeviceBuilder(physicalDevice);
		auto vkbDevice = deviceBuilder.build().value();

		m_LogicalDevice = vkbDevice.device;
		m_PhysicalDevice = vkbDevice.physical_device;

		m_Queue = vkbDevice.get_queue(vkb::QueueType::graphics).value();
		m_FamilyIndex = vkbDevice.get_queue_index(vkb::QueueType::graphics).value();
	}

	void VulkanContext::Destroy() const
	{
		vkDestroySurfaceKHR(m_Instance, m_WindowSurface, nullptr);
		vkDestroyDevice(m_LogicalDevice, nullptr);

		vkb::destroy_debug_utils_messenger(m_Instance, m_DebugUtils);
		vkDestroyInstance(m_Instance, nullptr);
	}
}