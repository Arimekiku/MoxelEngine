#pragma once

#include "vulkan_context.h"
#include "vulkan_allocator.h"
#include "vulkan_swapchain.h"
#include "vulkan_pipeline.h"
#include "vulkan_shader.h"
#include "vulkan_vertex_array.h"

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
		VulkanRenderer() = default;
		void Initialize(SDL_Window* window, const VkExtent2D& windowSize);

		void ImmediateSubmit(std::function<void(VkCommandBuffer freeBuffer)>&& function) const;

		void Draw();
		void Shutdown();

		static VulkanRenderer& Get() { return *s_Instance; }

		const std::shared_ptr<VulkanShader>& GetShader_TEST() { return m_GradientShader; }
		VulkanContext& GetContext() { return m_Context; }
		VulkanSwapchain& GetSwapchain() { return m_Swapchain; }
		VulkanCommandBuffer& GetCommandPool() { return m_CommandPool; }
		VulkanRendererSpecs& GetSpecifications() { return m_Specs; }
		const std::shared_ptr<VulkanImage>& GetFramebuffer() { return m_Framebuffer; }

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

		VulkanGraphicsPipeline m_MeshedPipeline;
		VulkanVertexArray m_Rectangle;

		VulkanComputePipeline m_GradientPipeline;
		VulkanShaderLibrary m_ShaderLibrary = VulkanShaderLibrary();
		std::shared_ptr<VulkanShader> m_GradientShader;
		std::shared_ptr<VulkanImage> m_Framebuffer;
	};
}
