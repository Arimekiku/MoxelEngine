#include "vulkan_pipeline.h"
#include "vulkan.h"
#include "scene/voxels/render_quad.h"
#include "engine/application.h"

#include <ranges>
#include <vulkan/vk_enum_string_helper.h>

namespace Moxel
{
	//
	// VulkanGraphicsPipeline::Builder
	//

	VulkanGraphicsPipeline::Builder& VulkanGraphicsPipeline::Builder::add_layout(const VkDescriptorSetLayout newLayout)
	{
		m_layouts.push_back(newLayout);

		return *this;
	}

	VulkanGraphicsPipeline::Builder& VulkanGraphicsPipeline::Builder::add_push_constant(const VkPushConstantRange pushConstant)
	{
		m_ranges.push_back(pushConstant);

		return *this;
	}

	VulkanGraphicsPipeline::Builder& VulkanGraphicsPipeline::Builder::with_fragment(VkPipelineShaderStageCreateInfo& fragment)
	{
		m_fragment = &fragment;

		return *this;
	}

	VulkanGraphicsPipeline::Builder& VulkanGraphicsPipeline::Builder::add_color_attachment(const VkFormat attachment)
	{
		for (const auto& colorAttachment: m_colorAttachments)
		{
			if (colorAttachment != attachment)
				continue;

			LOG_WARN("Color attachment format {0} is already in builder!", string_VkFormat(attachment));
			return *this;
		}

		m_colorAttachments.push_back(attachment);
		return *this;
	}

	VulkanGraphicsPipeline::Builder& VulkanGraphicsPipeline::Builder::with_vertex(VkPipelineShaderStageCreateInfo& vertex)
	{
		m_vertex = &vertex;

		return *this;
	}

	VulkanGraphicsPipeline::Builder& VulkanGraphicsPipeline::Builder::with_depth_attachment(VkFormat attachment)
	{
		m_depthAttachment = attachment;

		return *this;
	}

	std::unique_ptr<VulkanGraphicsPipeline> VulkanGraphicsPipeline::Builder::build()
	{
		// set pipeline graphics info
		auto graphicsInfo = VkPipelineLayoutCreateInfo();
		graphicsInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		graphicsInfo.setLayoutCount = m_layouts.size();
		graphicsInfo.pSetLayouts = m_layouts.data();
		graphicsInfo.pushConstantRangeCount = m_ranges.size();
		graphicsInfo.pPushConstantRanges = m_ranges.data();

		// set pipeline shader info
		if (m_fragment != nullptr)
			m_shaders.push_back(*m_fragment);

		if (m_vertex != nullptr)
			m_shaders.push_back(*m_vertex);

		// set pipeline rendering info
		auto renderingInfo = VkPipelineRenderingCreateInfo();
		renderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
		renderingInfo.pNext = nullptr;
		renderingInfo.colorAttachmentCount = m_colorAttachments.size();
		renderingInfo.pColorAttachmentFormats = m_colorAttachments.data();
		renderingInfo.depthAttachmentFormat = VK_FORMAT_D32_SFLOAT_S8_UINT;

		return std::make_unique<VulkanGraphicsPipeline>(graphicsInfo, renderingInfo, m_shaders);
	}

	//
	// VulkanGraphicsPipeline
	//

	VulkanGraphicsPipeline::VulkanGraphicsPipeline(VkPipelineLayoutCreateInfo graphicsInfo, VkPipelineRenderingCreateInfo renderingInfo, std::vector<VkPipelineShaderStageCreateInfo>& shaders)
	{
		const auto device = Application::get().get_context().get_logical_device();

		// set layout create info
		auto result = vkCreatePipelineLayout(device, &graphicsInfo, nullptr, &m_layout);
		VULKAN_CHECK(result);

		// set vertex input info
		auto bindingDescription = VkVertexInputBindingDescription();
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(VoxelVertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		auto attributeDescriptions = std::vector<VkVertexInputAttributeDescription>(2);
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32_UINT;
		attributeDescriptions[0].offset = offsetof(VoxelVertex, Position);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(VoxelVertex, Color);

		auto vertexInputInfo = VkPipelineVertexInputStateCreateInfo();
		vertexInputInfo.pNext = nullptr;
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
		vertexInputInfo.vertexAttributeDescriptionCount = attributeDescriptions.size();
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

		// set input assembly info
		auto inputAssemblyInfo = VkPipelineInputAssemblyStateCreateInfo();
		inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssemblyInfo.pNext = nullptr;
		inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

		// set viewport state
		auto viewportState = VkPipelineViewportStateCreateInfo();
		viewportState.pNext = nullptr;
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.scissorCount = 1;

		// set rasterization info
		auto rasterizationInfo = VkPipelineRasterizationStateCreateInfo();
		rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizationInfo.pNext = nullptr;
		rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizationInfo.cullMode = VK_CULL_MODE_FRONT_BIT;
		rasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
		rasterizationInfo.lineWidth = 1.0f;

		// TODO: currently not supported dynamic multisampling pipelines
		// multisampling
		auto multisamplingInfo = VkPipelineMultisampleStateCreateInfo();
		multisamplingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisamplingInfo.pNext = nullptr;
		multisamplingInfo.sampleShadingEnable = VK_FALSE;
		multisamplingInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisamplingInfo.minSampleShading = 1.0f;
		multisamplingInfo.alphaToCoverageEnable = VK_FALSE;
		multisamplingInfo.alphaToOneEnable = VK_FALSE;

		// blending
		auto colorBlendAttachment = VkPipelineColorBlendAttachmentState();
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_TRUE;
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

		auto colorBlending = VkPipelineColorBlendStateCreateInfo();
		colorBlending.pNext = nullptr;
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;

		// set depth stencil info
		auto depthStencilInfo = VkPipelineDepthStencilStateCreateInfo();
		depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencilInfo.pNext = nullptr;
		depthStencilInfo.depthTestEnable = VK_TRUE;
		depthStencilInfo.depthWriteEnable = VK_TRUE;
		depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
		depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
		depthStencilInfo.stencilTestEnable = VK_FALSE;
		depthStencilInfo.front = VkStencilOpState();
		depthStencilInfo.back = VkStencilOpState();
		depthStencilInfo.minDepthBounds = 0.f;
		depthStencilInfo.maxDepthBounds = 1.f;

		// build the actual pipeline
		auto pipelineInfo = VkGraphicsPipelineCreateInfo();
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.pNext = &renderingInfo;
		pipelineInfo.stageCount = static_cast<uint32_t>(shaders.size());
		pipelineInfo.pStages = shaders.data();
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssemblyInfo;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizationInfo;
		pipelineInfo.pMultisampleState = &multisamplingInfo;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDepthStencilState = &depthStencilInfo;
		pipelineInfo.layout = m_layout;

		constexpr VkDynamicState state[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

		auto dynamicInfo = VkPipelineDynamicStateCreateInfo();
		dynamicInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicInfo.pDynamicStates = &state[0];
		dynamicInfo.dynamicStateCount = 2;

		pipelineInfo.pDynamicState = &dynamicInfo;

		result = vkCreateGraphicsPipelines(device, nullptr, 1, &pipelineInfo, nullptr, &m_pipeline);
		VULKAN_CHECK(result);
	}

	VulkanGraphicsPipeline::~VulkanGraphicsPipeline()
	{
		const auto device = Application::get().get_context().get_logical_device();

		vkDestroyPipelineLayout(device, m_layout, nullptr);
		vkDestroyPipeline(device, m_pipeline, nullptr);
	}

	//
	// VulkanComputePipeline
	//

	VulkanComputePipeline::VulkanComputePipeline(const VulkanComputePipelineSpecs& specs)
	{
		m_specs = specs;
		const auto device = Application::get().get_context().get_logical_device();

		auto imageInfo = VkDescriptorImageInfo();
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
		imageInfo.imageView = specs.Framebuffer->ImageView;

		auto computeLayout = VkPipelineLayoutCreateInfo();
		computeLayout.pNext = nullptr;
		computeLayout.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		computeLayout.pSetLayouts = nullptr;
		computeLayout.setLayoutCount = 0;
		computeLayout.pPushConstantRanges = nullptr;
		computeLayout.pushConstantRangeCount = 0;

		if (specs.PushConstants.size > 0)
		{
			computeLayout.pPushConstantRanges = &specs.PushConstants;
			computeLayout.pushConstantRangeCount = 1;
		}

		auto result = vkCreatePipelineLayout(device, &computeLayout, nullptr, &m_layout);
		VULKAN_CHECK(result);

		auto computePipelineCreateInfo = VkComputePipelineCreateInfo();
		computePipelineCreateInfo.pNext = nullptr;
		computePipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		computePipelineCreateInfo.layout = m_layout;
		computePipelineCreateInfo.stage = specs.Compute->get_pipeline_create_info();

		result = vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &computePipelineCreateInfo, nullptr, &m_pipeline);
		VULKAN_CHECK(result);
	}

	void VulkanComputePipeline::destroy()
	{
		const auto device = Application::get().get_context().get_logical_device();

		m_specs.clear();

		vkDestroyPipelineLayout(device, m_layout, nullptr);
		vkDestroyPipeline(device, m_pipeline, nullptr);
	}
}
