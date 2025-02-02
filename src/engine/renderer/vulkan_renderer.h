#pragma once

#include "vulkan_swapchain.h"
#include "vulkan_pipeline.h"
#include "vulkan_shader.h"
#include "scene/voxels/chunk.h"

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
		static void free_resource_submit(std::function<void()>&& function) { s_deletionQueue.push_back(function); }

		static void prepare_frame();
		static void end_frame();

		static void render_chunk(const ChunkPosition chunkPosition, const std::shared_ptr<ChunkMesh>& chunk, const glm::mat4& cameraMat);

		static VulkanSwapchain& get_swapchain() { return s_renderData.Swapchain; }
		static VulkanCommandBuffer& get_command_pool() { return s_renderData.CommandPool; }
		static VulkanRendererSpecs& get_specifications() { return s_renderData.Specs; }

		static int get_current_frame_index() { return s_renderData.CurrentFrameIndex % 2; }
	private:
		struct RenderStaticData
		{
			CommandBufferData BufferData;
			int CurrentFrameIndex = 0;

			VulkanRendererSpecs Specs = VulkanRendererSpecs();
			VulkanSwapchain Swapchain = VulkanSwapchain();
			VulkanCommandBuffer CommandPool = VulkanCommandBuffer();

			std::unique_ptr<VulkanGraphicsPipeline> MeshedPipeline;

			std::unique_ptr<VulkanDescriptorPool> GlobalDescriptorPool;
			std::vector<VkDescriptorSet> GlobalSets;

			VulkanShaderLibrary ShaderLibrary = VulkanShaderLibrary();
			std::vector<std::shared_ptr<VulkanBufferUniform>> Uniforms;
		};

		static std::vector<std::function<void()>> s_deletionQueue;
		static RenderStaticData s_renderData;
	};
}