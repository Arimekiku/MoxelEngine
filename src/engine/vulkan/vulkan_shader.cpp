#include "vulkan_shader.h"
#include "vulkan.h"

#include <fstream>

namespace SDLarria 
{
	void DescriptorLayoutBuilder::AddBinding(uint32_t binding, VkDescriptorType type)
	{
		auto bind = VkDescriptorSetLayoutBinding();
		bind.binding = binding;
		bind.descriptorCount = 1;
		bind.descriptorType = type;

		m_Bindings.push_back(bind);
	}

	void DescriptorLayoutBuilder::Clear()
	{
		m_Bindings.clear();
	}

	VkDescriptorSetLayout DescriptorLayoutBuilder::Build(VkDevice device, const VkShaderStageFlags shaderStages, const VkDescriptorSetLayoutCreateFlags flags)
	{
		for (auto& bind : m_Bindings) 
		{
			bind.stageFlags |= shaderStages;
		}

		auto info = VkDescriptorSetLayoutCreateInfo();
		info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		info.pNext = nullptr;
		info.pBindings = m_Bindings.data();
		info.bindingCount = (uint32_t)m_Bindings.size();
		info.flags = flags;

		auto layout = VkDescriptorSetLayout();
		const auto result = vkCreateDescriptorSetLayout(device, &info, nullptr, &layout);
		VULKAN_CHECK(result);

		return layout;
	}

	VulkanShader::VulkanShader(const char* filePath, DescriptorAllocator& allocator)
	{
		const auto device = VulkanRenderer::Get().GetContext().GetLogicalDevice();
		const auto framebuffer = VulkanRenderer::Get().GetFramebuffer();

		std::ifstream file(filePath, std::ios::ate | std::ios::binary);
		LOG_ASSERT(file.is_open(), "Couldn't open file for shader!");

		const size_t fileSize = file.tellg();
		auto buffer = std::vector<uint32_t>(fileSize / sizeof(uint32_t));

		file.seekg(0);
		file.read((char*)buffer.data(), fileSize);
		file.close();

		auto createInfo = VkShaderModuleCreateInfo();
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.codeSize = buffer.size() * sizeof(uint32_t);
		createInfo.pCode = buffer.data();

		auto module = VkShaderModule();
		auto result = vkCreateShaderModule(device, &createInfo, nullptr, &module);
		VULKAN_CHECK(result);

		// create shader pipeline
		auto stageInfo = VkPipelineShaderStageCreateInfo();
		stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		stageInfo.pNext = nullptr;
		stageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
		stageInfo.module = module;
		stageInfo.pName = "main";

		auto builder = DescriptorLayoutBuilder();
		builder.AddBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
		m_DescriptorSetLayout = builder.Build(device, VK_SHADER_STAGE_COMPUTE_BIT);
		m_DescriptorSet = allocator.AllocateSet(m_DescriptorSetLayout);

		auto imgInfo = VkDescriptorImageInfo();
		imgInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
		imgInfo.imageView = framebuffer.GetImageView();

		auto drawImageWrite = VkWriteDescriptorSet();
		drawImageWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		drawImageWrite.pNext = nullptr;

		drawImageWrite.dstBinding = 0;
		drawImageWrite.dstSet = m_DescriptorSet;
		drawImageWrite.descriptorCount = 1;
		drawImageWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		drawImageWrite.pImageInfo = &imgInfo;

		vkUpdateDescriptorSets(device, 1, &drawImageWrite, 0, nullptr);

		auto computeLayout = VkPipelineLayoutCreateInfo();
		computeLayout.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		computeLayout.pNext = nullptr;
		computeLayout.pSetLayouts = &m_DescriptorSetLayout;
		computeLayout.setLayoutCount = 1;

		auto pushConstant = VkPushConstantRange();
		pushConstant.offset = 0;
		pushConstant.size = sizeof(ComputePushConstants);
		pushConstant.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

		computeLayout.pPushConstantRanges = &pushConstant;
		computeLayout.pushConstantRangeCount = 1;

		result = vkCreatePipelineLayout(device, &computeLayout, nullptr, &m_ShaderPipelineLayout);
		VULKAN_CHECK(result);

		auto computePipelineCreateInfo = VkComputePipelineCreateInfo();
		computePipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		computePipelineCreateInfo.pNext = nullptr;
		computePipelineCreateInfo.layout = m_ShaderPipelineLayout;
		computePipelineCreateInfo.stage = stageInfo;

		result = vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &computePipelineCreateInfo, nullptr, &m_ShaderPipeline);
		VULKAN_CHECK(result);

		vkDestroyShaderModule(device, stageInfo.module, nullptr);
	}

	void VulkanShader::Reload() const
	{
		const auto device = VulkanRenderer::Get().GetContext().GetLogicalDevice();
		const auto framebuffer = VulkanRenderer::Get().GetFramebuffer();

		auto imgInfo = VkDescriptorImageInfo();
		imgInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
		imgInfo.imageView = framebuffer.GetImageView();

		auto drawImageWrite = VkWriteDescriptorSet();
		drawImageWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		drawImageWrite.pNext = nullptr;
		drawImageWrite.dstBinding = 0;
		drawImageWrite.dstSet = m_DescriptorSet;
		drawImageWrite.descriptorCount = 1;
		drawImageWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		drawImageWrite.pImageInfo = &imgInfo;

		vkUpdateDescriptorSets(device, 1, &drawImageWrite, 0, nullptr);
	}

	void VulkanShader::Destroy() const
	{
		const auto device = VulkanRenderer::Get().GetContext().GetLogicalDevice();

		vkDestroyDescriptorSetLayout(device, m_DescriptorSetLayout, nullptr);
		vkDestroyPipelineLayout(device, m_ShaderPipelineLayout, nullptr);
		vkDestroyPipeline(device, m_ShaderPipeline, nullptr);
	}
}