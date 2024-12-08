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

	VkDescriptorSetLayout DescriptorLayoutBuilder::Build(VkDevice device, VkShaderStageFlags shaderStages, void* pNext, VkDescriptorSetLayoutCreateFlags flags)
	{
		for (auto& bind : m_Bindings) 
		{
			bind.stageFlags |= shaderStages;
		}

		auto info = VkDescriptorSetLayoutCreateInfo();
		info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		info.pNext = pNext;
		info.pBindings = m_Bindings.data();
		info.bindingCount = (uint32_t)m_Bindings.size();
		info.flags = flags;

		auto layout = VkDescriptorSetLayout();
		auto result = vkCreateDescriptorSetLayout(device, &info, nullptr, &layout);
		VULKAN_CHECK(result);

		return layout;
	}

	VulkanShader::VulkanShader(const char* filePath, VkDevice device)
	{
		std::ifstream file(filePath, std::ios::ate | std::ios::binary);
		LOG_ASSERT(file.is_open(), "Couldn't open file for shader!");

		size_t fileSize = (size_t)file.tellg();
		auto buffer = std::vector<uint32_t>(fileSize / sizeof(uint32_t));

		file.seekg(0);
		file.read((char*)buffer.data(), fileSize);
		file.close();

		VkShaderModuleCreateInfo createInfo = VkShaderModuleCreateInfo();
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.codeSize = buffer.size() * sizeof(uint32_t);
		createInfo.pCode = buffer.data();

		auto module = VkShaderModule();
		auto result = vkCreateShaderModule(device, &createInfo, nullptr, &module);
		VULKAN_CHECK(result);

		auto stageinfo = VkPipelineShaderStageCreateInfo();
		stageinfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		stageinfo.pNext = nullptr;
		stageinfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
		stageinfo.module = module;
		stageinfo.pName = "main";

		m_PipelineShaderStageInfo = stageinfo;
	}

	void VulkanShader::Unload(VkDevice device) const
	{
		if (m_PipelineShaderStageInfo.module) 
		{
			vkDestroyShaderModule(device, m_PipelineShaderStageInfo.module, nullptr);
			return;
		}

		LOG_WARN("Shader was already unloaded!");
	}
}