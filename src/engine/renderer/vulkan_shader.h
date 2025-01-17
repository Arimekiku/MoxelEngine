#pragma once

#include "vulkan_descriptors.h"

namespace Moxel
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

		void release();
		void destroy() const;

		const VkPipelineShaderStageCreateInfo& get_pipeline_create_info() const { return m_createInfo; }
		VkPipelineShaderStageCreateInfo& get_pipeline_create_info() { return m_createInfo; }

	private:
		VkPipelineShaderStageCreateInfo m_createInfo;

		friend class VulkanShaderLibrary;
	};

	class VulkanShaderLibrary
	{
	public:
		VulkanShaderLibrary() = default;

		const std::shared_ptr<VulkanShader>& get_shader(const int index) { return m_shaders[index]; }
		void destroy();

		void add(const std::shared_ptr<VulkanShader>& shader);

	private:
		std::vector<std::shared_ptr<VulkanShader>> m_shaders;
	};
}
