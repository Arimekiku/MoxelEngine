#pragma once

#include "vulkan_shader.h"
#include "vulkan_image.h"

#include <memory>

namespace Moxel
{
	struct VulkanGraphicsPipelineSpecs
	{
		std::shared_ptr<VulkanShader> Fragment;
		std::shared_ptr<VulkanShader> Vertex;
		std::shared_ptr<VulkanImage> Framebuffer;

		VkPrimitiveTopology Topology;
		VkPolygonMode PolygonMode;
		VkCullModeFlags CullMode;
		VkFrontFace FrontFace;
		VkPushConstantRange PushConstants;
		VkBool32 MultisamplingEnable = VK_FALSE;
		VkBool32 BlendEnable = VK_FALSE;
		VkBool32 DepthTest = VK_FALSE;

		void clear()
		{ 
			Fragment = nullptr;
			Vertex = nullptr;
			Framebuffer = nullptr;
		}
	};

	class VulkanGraphicsPipeline
	{
	public:
		VulkanGraphicsPipeline() = default;
		VulkanGraphicsPipeline(const VulkanGraphicsPipelineSpecs& specs, VkDescriptorSetLayout layout);

		void destroy();

		VkPipeline get_pipeline() const { return m_pipeline; }
		VkPipelineLayout get_pipeline_layout() const { return m_layout; }

	private:
		VulkanGraphicsPipelineSpecs m_specs;

		VkPipelineLayout m_layout = nullptr;
		VkPipeline m_pipeline = nullptr;
	};

	struct VulkanComputePipelineSpecs
	{
		std::shared_ptr<VulkanShader> Compute;
		std::shared_ptr<ImageAsset> Framebuffer;

		VkPushConstantRange PushConstants;

		void clear()
		{
			Compute = nullptr;
			Framebuffer = nullptr;
		}
	};

	class VulkanComputePipeline
	{
	public:
		VulkanComputePipeline() = default;
		VulkanComputePipeline(const VulkanComputePipelineSpecs& specs);

		void destroy();

		VkPipelineLayout get_layout() const { return m_layout; }
		VkPipeline get_pipeline() const { return m_pipeline; }

	private:
		VulkanComputePipelineSpecs m_specs;

		VkPipelineLayout m_layout = nullptr;
		VkPipeline m_pipeline = nullptr;
	};
}