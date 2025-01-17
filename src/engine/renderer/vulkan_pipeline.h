#pragma once

#include "vulkan_shader.h"
#include "vulkan_image.h"

namespace Moxel
{
	class VulkanGraphicsPipeline
	{
	public:
		class Builder
		{
		public:
			Builder() = default;

			Builder& add_layout(VkDescriptorSetLayout newLayout);
			Builder& add_push_constant(VkPushConstantRange pushConstant);
			Builder& add_color_attachment(VkFormat attachment);
			Builder& with_fragment(VkPipelineShaderStageCreateInfo& fragment);
			Builder& with_vertex(VkPipelineShaderStageCreateInfo& vertex);
			Builder& with_depth_attachment(VkFormat attachment);

			std::unique_ptr<VulkanGraphicsPipeline> build();
		private:
			std::vector<VkPushConstantRange> m_ranges;
			std::vector<VkDescriptorSetLayout> m_layouts;

			std::vector<VkFormat> m_colorAttachments;
			VkFormat m_depthAttachment;

			VkPipelineShaderStageCreateInfo* m_fragment = nullptr;
			VkPipelineShaderStageCreateInfo* m_vertex = nullptr;
			std::vector<VkPipelineShaderStageCreateInfo> m_shaders;
		};

		VulkanGraphicsPipeline(VkPipelineLayoutCreateInfo graphicsInfo, VkPipelineRenderingCreateInfo renderingInfo,
							   std::vector<VkPipelineShaderStageCreateInfo>& shaders);
		~VulkanGraphicsPipeline();

		VkPipeline get_pipeline() const { return m_pipeline; }
		VkPipelineLayout get_pipeline_layout() const { return m_layout; }

	private:
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