#pragma once

#include "vulkan_allocator.h"

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
		VkDescriptorSetLayout Build(VkDevice device, VkShaderStageFlags shaderStages, VkDescriptorSetLayoutCreateFlags flags = 0);

	private:
		std::vector<VkDescriptorSetLayoutBinding> m_Bindings;
	};

	class VulkanShader 
	{
	public:
		VulkanShader() = default;
		VulkanShader(const char* filePath, DescriptorAllocator& allocator);

		void Reload() const;
		void Destroy() const;

		const VkDescriptorSet* GetDescriptors() const { return &m_DescriptorSet; }
		VkPipeline GetShaderPipeline() const { return m_ShaderPipeline; }
		VkPipelineLayout GetShaderPipelineLayout() const { return m_ShaderPipelineLayout; }

	private:
		VkDescriptorSetLayout m_DescriptorSetLayout = nullptr;
		VkDescriptorSet m_DescriptorSet = nullptr;

		VkPipeline m_ShaderPipeline = nullptr;
		VkPipelineLayout m_ShaderPipelineLayout = nullptr;
	};
}
