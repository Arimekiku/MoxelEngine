#pragma once

#include "vulkan_allocator.h"
#include "glm/glm.hpp"

#include <vulkan/vulkan_core.h>
#include <vector>

namespace SDLarria
{
	struct ComputePushConstants_TEST
	{
		glm::vec4 data1;
		glm::vec4 data2;
		glm::vec4 data3;
		glm::vec4 data4;
	};

	enum class ShaderType
	{
		VERTEX,
		FRAGMENT,
		COMPUTE,
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
		VulkanShader(const char* filePath, DescriptorAllocator& allocator, ShaderType shaderType);
		~VulkanShader();

		void Release() const;

		const VkPipelineShaderStageCreateInfo& GetPipelineCreateInfo() const { return m_CreateInfo; }
		const VkDescriptorSetLayout& GetDescriptorSetLayout() const { return m_DescriptorSetLayout; }
		const VkDescriptorSet GetDescriptors() const { return m_DescriptorSet; }
		VkDescriptorSet GetDescriptors() { return m_DescriptorSet; }

		ComputePushConstants_TEST& GetPushConstants() { return m_PushConstants; }

	private:
		VkPipelineShaderStageCreateInfo m_CreateInfo;

		VkDescriptorSetLayout m_DescriptorSetLayout = nullptr;
		VkDescriptorSet m_DescriptorSet = nullptr;

		ComputePushConstants_TEST m_PushConstants = ComputePushConstants_TEST();
	};
}
