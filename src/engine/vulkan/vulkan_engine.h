#pragma once

#include "vulkan_allocator.h"
#include "vulkan_swapchain.h"
#include "vulkan_command_queue.h"
#include "vulkan_shader.h"

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
		DescriptorAllocator m_DescriptorAllocator = DescriptorAllocator();
		BufferAllocator m_BufferAllocator = BufferAllocator();
		VulkanInstance m_Instance = VulkanInstance();
		VulkanSwapchain m_Swapchain = VulkanSwapchain();
		VulkanCommandPool m_CommandPool = VulkanCommandPool();

		VkDescriptorSet Test_drawImageDescriptors;
		VkDescriptorSetLayout Test_drawImageDescriptorLayout;
		VkPipeline Test_gradientPipeline;
		VkPipelineLayout Test_gradientPipelineLayout;

		VkExtent2D m_WindowSize = VkExtent2D(0, 0);
		VulkanImage m_Framebuffer;
	};
}