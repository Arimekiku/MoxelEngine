#include "vulkan_shader.h"
#include "vulkan.h"

#include <fstream>
#include <ranges>

#include "engine/application.h"

namespace SDLarria 
{
	static std::vector<uint32_t> ReadFile(const char* filePath)
	{
		std::ifstream file(filePath, std::ios::ate | std::ios::binary);
		LOG_ASSERT(file.is_open(), "Couldn't open file for shader!");

		const uint32_t fileSize = file.tellg();
		auto buffer = std::vector<uint32_t>(fileSize / sizeof(uint32_t));

		file.seekg(0);
		file.read(reinterpret_cast<char*>(buffer.data()), fileSize);
		file.close();

		return buffer;
	}

	VulkanShader::VulkanShader(const char* filePath, const ShaderType shaderType)
	{
		const auto device = Application::Get().GetContext().GetLogicalDevice();
		const auto buffer = ReadFile(filePath);

		auto createInfo = VkShaderModuleCreateInfo();
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.codeSize = buffer.size() * sizeof(uint32_t);
		createInfo.pCode = buffer.data();

		auto module = VkShaderModule();
		const auto result = vkCreateShaderModule(device, &createInfo, nullptr, &module);
		VULKAN_CHECK(result);

		// create shader pipeline info
		auto shaderStageFlag = VkShaderStageFlagBits();
		switch (shaderType)
		{
			case ShaderType::VERTEX: shaderStageFlag = VK_SHADER_STAGE_VERTEX_BIT; break;
			case ShaderType::FRAGMENT: shaderStageFlag = VK_SHADER_STAGE_FRAGMENT_BIT; break;
			case ShaderType::COMPUTE: shaderStageFlag = VK_SHADER_STAGE_COMPUTE_BIT; break;
		}

		m_CreateInfo = VkPipelineShaderStageCreateInfo();
		m_CreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		m_CreateInfo.pNext = nullptr;
		m_CreateInfo.stage = shaderStageFlag;
		m_CreateInfo.module = module;
		m_CreateInfo.pName = "main";
	}

	void VulkanShader::Destroy() const
	{
		const auto device = Application::Get().GetContext().GetLogicalDevice();

		if (m_CreateInfo.module)
		{
			vkDestroyShaderModule(device, m_CreateInfo.module, nullptr);
		}
	}

	void VulkanShader::Release()
	{
		const auto device = Application::Get().GetContext().GetLogicalDevice();

		if (m_CreateInfo.module)
		{
			vkDestroyShaderModule(device, m_CreateInfo.module, nullptr);

			m_CreateInfo.module = nullptr;
		}
	}

	void VulkanShaderLibrary::Destroy()
	{
		for (const auto& shader: m_Shaders)
		{
			shader->Destroy();
		}

		m_Shaders.clear();
	}

	void VulkanShaderLibrary::Add(const std::shared_ptr<VulkanShader>& shader)
	{
		m_Shaders.emplace_back(shader);
	}
}
