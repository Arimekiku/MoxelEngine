#pragma once

#include <memory>
#include <unordered_map>
#include <vector>
#include <vulkan/vulkan_core.h>

#include "vulkan_buffer_uniform.h"

namespace SDLarria
{
	class VulkanDescriptorPool
	{
	public:
		class Builder
		{
		public:
			Builder() = default;

			Builder& AddPoolSize(VkDescriptorType descriptorType, uint32_t count);
			Builder& SetCreateFlags(VkDescriptorPoolCreateFlags flags);
			Builder& SetMaxSets(uint32_t count);

			std::unique_ptr<VulkanDescriptorPool> Build() const;

		private:
			std::vector<VkDescriptorPoolSize> m_PoolSizes;
			VkDescriptorPoolCreateFlags m_PoolFlags = 0;

			uint32_t m_MaxSets = 1000;
		};

		VulkanDescriptorPool(uint32_t maxSets, VkDescriptorPoolCreateFlags poolFlags, const std::vector<VkDescriptorPoolSize>& poolSizes);
		~VulkanDescriptorPool();

		bool AllocateDescriptor(VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptorSet) const;
		void FreeDescriptors(const std::vector<VkDescriptorSet>& descriptors) const;
		void ResetPool() const;

	private:
		VkDescriptorPool m_DescriptorPool = nullptr;

		friend class DescriptorWriter;
	};

	class VulkanDescriptorSetLayout
	{
	public:
		class Builder
		{
		public:
			Builder() = default;

			Builder& AddBinding(uint32_t binding, VkDescriptorType descriptorType, VkShaderStageFlags stageFlags, uint32_t count = 1);
			std::unique_ptr<VulkanDescriptorSetLayout> Build() const;

		private:
			std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> m_Bindings;
		};

		VulkanDescriptorSetLayout(const std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding>& bindings);
		~VulkanDescriptorSetLayout();

		VkDescriptorSetLayout GetDescriptorSetLayout() const { return m_DescriptorSetLayout; }

	private:
		VkDescriptorSetLayout m_DescriptorSetLayout = nullptr;
		std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> m_Bindings;

		friend class DescriptorWriter;
	};

	class DescriptorWriter
	{
	public:
		DescriptorWriter(VulkanDescriptorSetLayout& layout, VulkanDescriptorPool& pool)
			: m_SetLayout(layout), m_Pool(pool) { }

		DescriptorWriter& WriteBuffer(uint32_t binding, const VkDescriptorBufferInfo& bufferInfo);
		DescriptorWriter& WriteImage(uint32_t binding, const VkDescriptorImageInfo& imageInfo);

		bool Build(VkDescriptorSet& set);
		void Overwrite(const VkDescriptorSet& set);

	private:
		VulkanDescriptorSetLayout& m_SetLayout;
		VulkanDescriptorPool& m_Pool;

		std::vector<VkWriteDescriptorSet> m_DescriptorSets;
	};
}
