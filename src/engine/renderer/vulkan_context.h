#pragma once

#include <vulkan/vulkan_core.h>
#include <SDL3/SDL_video.h>

namespace Moxel
{
	class VulkanContext
	{
	public:
		VulkanContext() = default;

		void initialize(SDL_Window* window);
		void destroy() const;

		VkInstance get_instance() const { return m_instance; }
		VkPhysicalDevice get_physical_device() const { return m_physicalDevice; }
		VkDevice get_logical_device() const { return m_logicalDevice; }
		VkSurfaceKHR get_window_surface() const { return m_windowSurface; }

		VkQueue get_render_queue() const { return m_queue; }
		uint32_t get_queue_family_index() const { return m_familyIndex; }

	private:
		VkSurfaceKHR m_windowSurface = nullptr;

		VkInstance m_instance = nullptr;
		VkDebugUtilsMessengerEXT m_debugUtils = nullptr;
		VkPhysicalDevice m_physicalDevice = nullptr;
		VkDevice m_logicalDevice = nullptr;

		VkQueue m_queue = nullptr;
		uint32_t m_familyIndex = 0;
	};
}