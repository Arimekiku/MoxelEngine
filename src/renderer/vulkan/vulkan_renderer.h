#pragma once

#include "vulkan_swapchain.h"
#include "vulkan_pipeline.h"
#include "vulkan_shader.h"
#include "engine/chunk.h"

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

		static void initialize(const VkExtent2D& windowSize);
		static void shutdown();

		static void immediate_submit(std::function<void(VkCommandBuffer freeBuffer)>&& function);
		static void queue_resource_free(const std::shared_ptr<VulkanVertexArray>& vertexArray)
		{
			s_deletionQueue.push_back(vertexArray);
		}

		static void prepare_frame();
		static void end_frame();

		static void render_chunk(const ChunkPosition chunkPosition, const std::shared_ptr<ChunkMesh>& chunk, const glm::mat4& cameraMat);

		static VulkanSwapchain& get_swapchain() { return s_renderData.m_swapchain; }
		static VulkanCommandBuffer& get_command_pool() { return s_renderData.m_commandPool; }
		static VulkanRendererSpecs& get_specifications() { return s_renderData.m_specs; }

		static int get_current_frame_index() { return s_renderData.m_currentFrameIndex % 2; }
	private:
		struct RenderStaticData
		{
			CommandBufferData m_bufferData;
			int m_currentFrameIndex = 0;

			VulkanRendererSpecs m_specs = VulkanRendererSpecs();
			VulkanSwapchain m_swapchain = VulkanSwapchain();
			VulkanCommandBuffer m_commandPool = VulkanCommandBuffer();

			VulkanGraphicsPipeline m_meshedPipeline;

			std::unique_ptr<VulkanDescriptorPool> m_globalDescriptorPool;
			std::vector<VkDescriptorSet> m_globalSets;

			VulkanShaderLibrary m_shaderLibrary = VulkanShaderLibrary();
			std::vector<std::shared_ptr<VulkanBufferUniform>> m_uniforms;
		};

		static std::vector<std::shared_ptr<VulkanVertexArray>> s_deletionQueue;
		static RenderStaticData s_renderData;
	};
}