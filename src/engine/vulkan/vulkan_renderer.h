#pragma once

#include "vulkan_context.h"
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
		VulkanRenderer();

		static void Initialize(const VkExtent2D& windowSize);
		static void Shutdown();

		static void ImmediateSubmit(std::function<void(VkCommandBuffer freeBuffer)>&& function);

		static void PrepareFrame();
		static void EndFrame();

		static void RenderVertexArray(const std::shared_ptr<VulkanVertexArray>& vertexArray);

		static VulkanRenderer& Get() { return *s_Instance; }

		VulkanSwapchain& GetSwapchain() { return s_RenderData.m_Swapchain; }
		VulkanCommandBuffer& GetCommandPool() { return s_RenderData.m_CommandPool; }
		VulkanRendererSpecs& GetSpecifications() { return s_RenderData.m_Specs; }
		const std::shared_ptr<VulkanImage>& GetFramebuffer() { return s_RenderData.m_Framebuffer; }

		int GetCurrentFrameIndex() const { return s_RenderData.m_CurrentFrameIndex % 2; }
	private:
		struct RenderStaticData
		{
			CommandBufferData m_BufferData;
			int m_CurrentFrameIndex = 0;

			VulkanRendererSpecs m_Specs = VulkanRendererSpecs();
			VulkanSwapchain m_Swapchain = VulkanSwapchain();
			VulkanCommandBuffer m_CommandPool = VulkanCommandBuffer();

			VulkanGraphicsPipeline m_MeshedPipeline;

			std::unique_ptr<VulkanDescriptorPool> m_GlobalDescriptorPool;
			std::vector<VkDescriptorSet> m_GlobalSets;

			VulkanShaderLibrary m_ShaderLibrary = VulkanShaderLibrary();
			std::shared_ptr<VulkanImage> m_Framebuffer;
			std::vector<std::shared_ptr<VulkanBufferUniform>> m_Uniforms;
		};

		static VulkanRenderer* s_Instance;
		static RenderStaticData s_RenderData;
	};
}
