#pragma once

#include "vulkan_instance.h"
#include "vulkan_swapchain.h"
#include "vulkan_command_queue.h"

#include <SDL3/SDL.h>

namespace SDLarria 
{
	class VulkanEngine 
	{
	public:
		void Initialize(SDL_Window* window, VkExtent2D windowSize);

		void Draw();

		void Shutdown();

	private:
		VulkanInstance m_Instance = VulkanInstance();
		VulkanSwapchain m_Swapchain = VulkanSwapchain();
		VulkanCommandPool m_CommandPool = VulkanCommandPool();
	};
}