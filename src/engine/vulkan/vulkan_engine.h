#pragma once

#include "vulkan_swapchain.h"
#include "vulkan_command_queue.h"

#include <SDL3/SDL.h>
#include <vulkan/vulkan_core.h>

namespace SDLarria {
	class VulkanEngine {
	public:
		void Initialize(SDL_Window* window, VkExtent2D windowSize);

		void Draw();

		void Shutdown();

	private:
		VkInstance m_Instance = nullptr;
		VkDebugUtilsMessengerEXT m_DebugUtils = nullptr;
		VkPhysicalDevice m_PhysicalDevice = nullptr;
		VkDevice m_LogicalDevice = nullptr;
		VkSurfaceKHR m_WindowSurface = nullptr;

		VulkanSwapchain m_Swapchain = VulkanSwapchain();
		VulkanCommandPool m_CommandPool = VulkanCommandPool();
	};
}