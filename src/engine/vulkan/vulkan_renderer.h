#pragma once

#include "vulkan_context.h"
#include "vulkan_allocator.h"
#include "vulkan_swapchain.h"

#include <SDL3/SDL.h>

#include "vulkan_shader.h"

namespace SDLarria 
{
	struct VulkanRendererSpecs 
	{
		int FRAMES_IN_FLIGHT = 2;
	};

	class VulkanRenderer 
	{
	public:
		void Initialize(SDL_Window* window, const VkExtent2D& windowSize);

		void Draw();
		void Shutdown();

		static VulkanRenderer& Get() { return *s_Instance; }

		VulkanShader& GetShader_TEST() { return m_GradientShader; }
		VulkanContext GetContext() const { return m_Context; }
		VulkanSwapchain GetSwapchain() const { return m_Swapchain; }
		VulkanCommandBuffer GetCommandPool() const { return m_CommandPool; }
		VulkanRendererSpecs GetSpecifications() const { return m_Specs; }
		VulkanImage GetFramebuffer() const { return m_Framebuffer; }

		int GetCurrentFrameIndex() const { return m_CurrentFrameIndex % 2; }
	private:
		static VulkanRenderer* s_Instance;
		int m_CurrentFrameIndex = 0;

		VulkanRendererSpecs m_Specs = VulkanRendererSpecs();
		VulkanContext m_Context = VulkanContext();
		DescriptorAllocator m_DescriptorAllocator = DescriptorAllocator();
		BufferAllocator m_BufferAllocator = BufferAllocator();
		VulkanSwapchain m_Swapchain = VulkanSwapchain();
		VulkanCommandBuffer m_CommandPool = VulkanCommandBuffer();

		VulkanShader m_GradientShader;
		VulkanImage m_Framebuffer;
	};
}
