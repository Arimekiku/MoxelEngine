#pragma once

#include <vulkan/vulkan_core.h>
#include <vector>

#include "vulkan_descriptors.h"

namespace SDLarria
{
	enum class ShaderType
	{
		VERTEX,
		FRAGMENT,
		COMPUTE,
	};

	class VulkanShader
	{
	public:
		VulkanShader() = default;
		VulkanShader(const char* filePath, ShaderType shaderType);

		void Release();
		void Destroy() const;

		const VkPipelineShaderStageCreateInfo& GetPipelineCreateInfo() const { return m_CreateInfo; }

	private:
		VkPipelineShaderStageCreateInfo m_CreateInfo;

		friend class VulkanShaderLibrary;
	};

	class VulkanShaderLibrary
	{
	public:
		VulkanShaderLibrary() = default;
		~VulkanShaderLibrary() = default;

		const std::shared_ptr<VulkanShader>& GetShader(const int index) { return m_Shaders[index]; }
		void Destroy();

		void Add(const std::shared_ptr<VulkanShader>& shader);

	private:
		std::vector<std::shared_ptr<VulkanShader>> m_Shaders;
	};
}
