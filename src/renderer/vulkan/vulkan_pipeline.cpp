#include "vulkan_pipeline.h"
#include "vulkan.h"
#include "vulkan_buffer_vertex_array.h"
#include "renderer/application.h"

namespace Moxel
{
	VulkanGraphicsPipeline::VulkanGraphicsPipeline(const VulkanGraphicsPipelineSpecs& specs, VkDescriptorSetLayout layout)
	{
		m_Specs = specs;
		const auto device = Application::Get().GetContext().GetLogicalDevice();
		const auto shaderInfo = std::vector
		{
			specs.Fragment->GetPipelineCreateInfo(),
			specs.Vertex->GetPipelineCreateInfo(),
		};

		// set layout create info
		auto graphicsInfo = VkPipelineLayoutCreateInfo();
		graphicsInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		graphicsInfo.setLayoutCount = layout ? 1 : 0;
		graphicsInfo.pSetLayouts = &layout;
		graphicsInfo.pushConstantRangeCount = 0;
		graphicsInfo.pPushConstantRanges = nullptr;

		if (specs.PushConstants.size > 0)
		{
			graphicsInfo.pushConstantRangeCount = 1;
			graphicsInfo.pPushConstantRanges = &specs.PushConstants;
		}

		auto result = vkCreatePipelineLayout(device, &graphicsInfo, nullptr, &m_Layout);
		VULKAN_CHECK(result);

		// set pipeline rendering info
		auto renderingInfo = VkPipelineRenderingCreateInfo();
		renderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
		renderingInfo.pNext = nullptr;
		renderingInfo.colorAttachmentCount = 1;
		renderingInfo.pColorAttachmentFormats = specs.Framebuffer->GetImageFormat();
		renderingInfo.depthAttachmentFormat = VK_FORMAT_D32_SFLOAT;

		// set vertex input info
		auto bindingDescription = VkVertexInputBindingDescription();
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		auto attributeDescriptions = std::vector<VkVertexInputAttributeDescription>(2);
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, Position);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, Color);

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
		inputAssemblyInfo.topology = specs.Topology;
		inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

		// set viewport state
		auto viewportState = VkPipelineViewportStateCreateInfo();
		viewportState.pNext = nullptr;
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.scissorCount = 1;

		// set rasterizer info
		auto rasterizationInfo = VkPipelineRasterizationStateCreateInfo();
		rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizationInfo.pNext = nullptr;
		rasterizationInfo.polygonMode = specs.PolygonMode;
		rasterizationInfo.cullMode = specs.CullMode;
		rasterizationInfo.frontFace = specs.FrontFace;
		rasterizationInfo.lineWidth = 1.f;

		// TODO: currently not supported dynamic multisampling pipelines
		// multisampling
		auto multisampleInfo = VkPipelineMultisampleStateCreateInfo();
		multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampleInfo.pNext = nullptr;
		multisampleInfo.sampleShadingEnable = VK_FALSE;
		multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampleInfo.minSampleShading = 1.0f;
		multisampleInfo.alphaToCoverageEnable = VK_FALSE;
		multisampleInfo.alphaToOneEnable = VK_FALSE;

		// TODO: currently not supported dynamic blending pipelines
		// setup dummy color blending. We aren't using transparent objects yet
		// the blending is just "no blend", but we do write to the color attachment
		auto colorBlendAttachment = VkPipelineColorBlendAttachmentState();
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;

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
		pipelineInfo.stageCount = static_cast<uint32_t>(shaderInfo.size());
		pipelineInfo.pStages = shaderInfo.data();
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssemblyInfo;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizationInfo;
		pipelineInfo.pMultisampleState = &multisampleInfo;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDepthStencilState = &depthStencilInfo;
		pipelineInfo.layout = m_Layout;

		constexpr VkDynamicState state[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

		auto dynamicInfo = VkPipelineDynamicStateCreateInfo();
		dynamicInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicInfo.pDynamicStates = &state[0];
		dynamicInfo.dynamicStateCount = 2;

		pipelineInfo.pDynamicState = &dynamicInfo;

		result = vkCreateGraphicsPipelines(device, nullptr, 1, &pipelineInfo, nullptr, &m_Pipeline);
		VULKAN_CHECK(result);
	}

	void VulkanGraphicsPipeline::Destroy()
	{
		const auto device = Application::Get().GetContext().GetLogicalDevice();

		m_Specs.Clear();

		vkDestroyPipelineLayout(device, m_Layout, nullptr);
		vkDestroyPipeline(device, m_Pipeline, nullptr);
	}

	VulkanComputePipeline::VulkanComputePipeline(const VulkanComputePipelineSpecs& specs)
	{
		m_Specs = specs;
		const auto device = Application::Get().GetContext().GetLogicalDevice();

		auto imageInfo = VkDescriptorImageInfo();
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
		imageInfo.imageView = specs.Framebuffer->GetImageView();

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

		auto result = vkCreatePipelineLayout(device, &computeLayout, nullptr, &m_Layout);
		VULKAN_CHECK(result);

		auto computePipelineCreateInfo = VkComputePipelineCreateInfo();
		computePipelineCreateInfo.pNext = nullptr;
		computePipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		computePipelineCreateInfo.layout = m_Layout;
		computePipelineCreateInfo.stage = specs.Compute->GetPipelineCreateInfo();

		result = vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &computePipelineCreateInfo, nullptr, &m_Pipeline);
		VULKAN_CHECK(result);
	}

	void VulkanComputePipeline::Destroy()
	{
		const auto device = Application::Get().GetContext().GetLogicalDevice();

		m_Specs.Clear();

		vkDestroyPipelineLayout(device, m_Layout, nullptr);
		vkDestroyPipeline(device, m_Pipeline, nullptr);
	}
}
