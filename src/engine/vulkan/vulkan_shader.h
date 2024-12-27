#pragma once

#include "vulkan_allocator.h"
#include "glm/glm.hpp"

#include <vulkan/vulkan_core.h>
#include <vector>

namespace SDLarria
{
	struct ComputePushConstants
	{
		glm::vec4 data1;
		glm::vec4 data2;
		glm::vec4 data3;
		glm::vec4 data4;
	};

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

		ComputePushConstants& GetPushConstants() { return m_PushConstants; }
		const VkDescriptorSet* GetDescriptors() const { return &m_DescriptorSet; }
		VkPipeline GetShaderPipeline() const { return m_ShaderPipeline; }
		VkPipelineLayout GetShaderPipelineLayout() const { return m_ShaderPipelineLayout; }

	private:
		VkDescriptorSetLayout m_DescriptorSetLayout = nullptr;
		VkDescriptorSet m_DescriptorSet = nullptr;

		VkPipeline m_ShaderPipeline = nullptr;
		VkPipelineLayout m_ShaderPipelineLayout = nullptr;

		ComputePushConstants m_PushConstants = ComputePushConstants();
	};
}
