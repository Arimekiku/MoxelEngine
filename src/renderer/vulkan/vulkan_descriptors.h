#pragma once

#include <memory>
#include <unordered_map>
#include <vector>
#include <vulkan/vulkan_core.h>

#include "vulkan_buffer_uniform.h"

namespace Moxel
{
	class VulkanDescriptorPool
	{
	public:
		class Builder
		{
		public:
			Builder() = default;

			Builder& add_pool_size(VkDescriptorType descriptorType, uint32_t count);
			Builder& set_create_flags(VkDescriptorPoolCreateFlags flags);
			Builder& set_max_sets(uint32_t count);

			std::unique_ptr<VulkanDescriptorPool> build() const;

		private:
			std::vector<VkDescriptorPoolSize> m_poolSizes;
			VkDescriptorPoolCreateFlags m_poolFlags = 0;

			uint32_t m_maxSets = 1000;
		};

		VulkanDescriptorPool(uint32_t maxSets, VkDescriptorPoolCreateFlags poolFlags, const std::vector<VkDescriptorPoolSize>& poolSizes);
		~VulkanDescriptorPool();

		bool allocate_descriptor(VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptorSet) const;
		void free_descriptors(const std::vector<VkDescriptorSet>& descriptors) const;
		void reset_pool() const;

	private:
		VkDescriptorPool m_descriptorPool = nullptr;

		friend class DescriptorWriter;
	};

	class VulkanDescriptorSetLayout
	{
	public:
		class Builder
		{
		public:
			Builder() = default;

			Builder& add_binding(uint32_t binding, VkDescriptorType descriptorType, VkShaderStageFlags stageFlags, uint32_t count = 1);
			std::unique_ptr<VulkanDescriptorSetLayout> build() const;

		private:
			std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> m_bindings;
		};

		VulkanDescriptorSetLayout(const std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding>& bindings);
		~VulkanDescriptorSetLayout();

		VkDescriptorSetLayout get_descriptor_set_layout() const { return m_descriptorSetLayout; }

	private:
		VkDescriptorSetLayout m_descriptorSetLayout = nullptr;
		std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> m_bindings;

		friend class DescriptorWriter;
	};

	class DescriptorWriter
	{
	public:
		DescriptorWriter(VulkanDescriptorSetLayout& layout, VulkanDescriptorPool& pool)
			: m_setLayout(layout), m_pool(pool) { }

		DescriptorWriter& write_buffer(uint32_t binding, const VkDescriptorBufferInfo& bufferInfo);
		DescriptorWriter& write_image(uint32_t binding, const VkDescriptorImageInfo& imageInfo);

		bool build(VkDescriptorSet& set);
		void overwrite(const VkDescriptorSet& set);

	private:
		VulkanDescriptorSetLayout& m_setLayout;
		VulkanDescriptorPool& m_pool;

		std::vector<VkWriteDescriptorSet> m_descriptorSets;
	};
}
