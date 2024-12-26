#pragma once

#include "vulkan_allocator.h"
#include "vulkan_swapchain.h"

#include <SDL3/SDL.h>

namespace SDLarria 
{
	struct VulkanRendererSpecs 
	{
		int FRAMES_IN_FLIGHT = 2;
	};

	class VulkanRenderer 
	{
	public:
		void Initialize(SDL_Window* window, VkExtent2D windowSize);

		void Draw();
		void Shutdown();

		static VulkanRenderer& Get() { return *s_Instance; }

		VulkanInstance GetInstance() const { return m_Instance; }
		VulkanSwapchain GetSwapchain() const { return m_Swapchain; }
		VulkanCommandBuffer GetCommandPool() const { return m_CommandPool; }
		VulkanRendererSpecs GetSpecifications() const { return m_Specs; }

		int GetCurrentFrameIndex() const { return m_CurrentFrameIndex % 2; }

		void ResizeTest();
	private:
		void ComputePipelineTest();

		static VulkanRenderer* s_Instance;
		int m_CurrentFrameIndex = 0;

		VulkanRendererSpecs m_Specs = VulkanRendererSpecs();
		VulkanInstance m_Instance = VulkanInstance();
		DescriptorAllocator m_DescriptorAllocator = DescriptorAllocator();
		BufferAllocator m_BufferAllocator = BufferAllocator();
		VulkanSwapchain m_Swapchain = VulkanSwapchain();
		VulkanCommandBuffer m_CommandPool = VulkanCommandBuffer();

		VkDescriptorSet Test_drawImageDescriptors;
		VkDescriptorSetLayout Test_drawImageDescriptorLayout;
		VkPipeline Test_gradientPipeline;
		VkPipelineLayout Test_gradientPipelineLayout;

		VkExtent2D m_WindowSize = VkExtent2D(0, 0);
		VulkanImage m_Framebuffer;
	};
}