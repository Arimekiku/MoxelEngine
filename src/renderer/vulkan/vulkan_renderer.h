#pragma once

#include "vulkan_swapchain.h"
#include "vulkan_pipeline.h"
#include "vulkan_shader.h"
#include "engine/render_mesh.h"
#include "engine/chunk.h"

#include <deque>

namespace Moxel
{
	struct VulkanRendererSpecs 
	{
		int FRAMES_IN_FLIGHT = 2;
	};

	class VulkanRenderer
	{
	public:
		VulkanRenderer() = delete;

		static void Initialize(const VkExtent2D& windowSize);
		static void Shutdown();

		static void ImmediateSubmit(std::function<void(VkCommandBuffer freeBuffer)>&& function);
		static void QueueResourceFree(const std::shared_ptr<VulkanVertexArray>& vertexArray)
		{
			s_DeletionQueue.push_back(vertexArray);
		}

		static void PrepareFrame();
		static void EndFrame();

		static void RenderVertexArray(const std::shared_ptr<RenderMesh>& mesh, const glm::mat4& cameraMat);
		static void RenderChunk(glm::mat4 trs, std::shared_ptr<Chunk>& chunk, const glm::mat4& cameraMat);

		static VulkanSwapchain& GetSwapchain() { return s_RenderData.m_Swapchain; }
		static VulkanCommandBuffer& GetCommandPool() { return s_RenderData.m_CommandPool; }
		static VulkanRendererSpecs& GetSpecifications() { return s_RenderData.m_Specs; }

		static int GetCurrentFrameIndex() { return s_RenderData.m_CurrentFrameIndex % 2; }
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
			std::vector<std::shared_ptr<VulkanBufferUniform>> m_Uniforms;
		};

		static std::vector<std::shared_ptr<VulkanVertexArray>> s_DeletionQueue;
		static RenderStaticData s_RenderData;
	};
}
