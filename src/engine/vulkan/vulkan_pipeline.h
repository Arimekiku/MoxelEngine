#pragma once

#include <memory>

#include "vulkan_shader.h"

namespace SDLarria 
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
		VkBool32 MultisampleEnable = VK_FALSE;
		VkBool32 BlendEnable = VK_FALSE;
		VkBool32 DepthTest = VK_FALSE;
	};

	class VulkanGraphicsPipeline
	{
	public:
		VulkanGraphicsPipeline() = default;
		VulkanGraphicsPipeline(const VulkanGraphicsPipelineSpecs& specs);
		~VulkanGraphicsPipeline();

		void Destroy() const;

		const VkPipeline GetPipeline() const { return m_Pipeline; }
		const VkPipelineLayout GetPipelineLayout() const { return m_Layout; }

	private:
		VulkanGraphicsPipelineSpecs m_Specs;

		VkPipelineLayout m_Layout = nullptr;
		VkPipeline m_Pipeline = nullptr;
	};

	struct VulkanComputePipelineSpecs
	{
		std::shared_ptr<VulkanShader> Compute;
		std::shared_ptr<VulkanImage> Framebuffer;

		VkPushConstantRange PushConstants;
	};

	class VulkanComputePipeline
	{
	public:
		VulkanComputePipeline() = default;
		VulkanComputePipeline(const VulkanComputePipelineSpecs& specs);

		void Destroy() const;
		void Reload() const;

		VkPipelineLayout GetLayout() const { return m_Layout; }
		VkPipeline GetPipeline() const { return m_Pipeline; }

	private:
		VulkanComputePipelineSpecs m_Specs;

		VkPipelineLayout m_Layout = nullptr;
		VkPipeline m_Pipeline = nullptr;
	};
}
