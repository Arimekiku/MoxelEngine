#pragma once

#include <vulkan/vulkan_core.h>
#include <vector>

namespace SDLarria
{
	class DescriptorLayoutBuilder 
	{
	public:
		DescriptorLayoutBuilder() = default;

		void AddBinding(uint32_t binding, VkDescriptorType type);
		void Clear();
		VkDescriptorSetLayout Build(VkDevice device, VkShaderStageFlags shaderStages, void* pNext = nullptr, VkDescriptorSetLayoutCreateFlags flags = 0);

	private:
		std::vector<VkDescriptorSetLayoutBinding> m_Bindings;
	};

	class VulkanShader 
	{
	public:
		VulkanShader(const char* filePath, VkDevice device);
		
		void Unload(VkDevice device) const;

		VkPipelineShaderStageCreateInfo GetRawShader() const { return m_PipelineShaderStageInfo; }

	private:
		VkPipelineShaderStageCreateInfo m_PipelineShaderStageInfo;
	};
}