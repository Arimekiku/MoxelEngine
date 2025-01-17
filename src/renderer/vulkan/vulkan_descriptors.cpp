#include "vulkan_descriptors.h"
#include "vulkan.h"
#include "renderer/application.h"

#include <ranges>

namespace Moxel
{
	//
	// VulkanDescriptorPool::Builder
	//

	VulkanDescriptorPool::Builder &VulkanDescriptorPool::Builder::add_pool_size(const VkDescriptorType descriptorType, const uint32_t count)
	{
		m_poolSizes.push_back(VkDescriptorPoolSize(descriptorType, count));

		return *this;
	}

	VulkanDescriptorPool::Builder &VulkanDescriptorPool::Builder::with_create_flags(const VkDescriptorPoolCreateFlags flags)
	{
		m_poolFlags = flags;

		return *this;
	}

	VulkanDescriptorPool::Builder &VulkanDescriptorPool::Builder::with_max_sets(const uint32_t count)
	{
		m_maxSets = count;

		return *this;
	}

	std::unique_ptr<VulkanDescriptorPool> VulkanDescriptorPool::Builder::build() const
	{
		return std::make_unique<VulkanDescriptorPool>(m_maxSets, m_poolFlags, m_poolSizes);
	}

	//
	// VulkanDescriptorPool
	//

	VulkanDescriptorPool::VulkanDescriptorPool(const uint32_t maxSets, const VkDescriptorPoolCreateFlags poolFlags, const std::vector<VkDescriptorPoolSize>& poolSizes)
	{
		const auto device = Application::get().get_context().get_logical_device();

		auto descriptorPoolInfo = VkDescriptorPoolCreateInfo();
		descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		descriptorPoolInfo.pPoolSizes = poolSizes.data();
		descriptorPoolInfo.maxSets = maxSets;
		descriptorPoolInfo.flags = poolFlags;

		const auto result = vkCreateDescriptorPool(device, &descriptorPoolInfo, nullptr, &m_descriptorPool);
		VULKAN_CHECK(result);
	}

	VulkanDescriptorPool::~VulkanDescriptorPool()
	{
		const auto device = Application::get().get_context().get_logical_device();

		vkDestroyDescriptorPool(device, m_descriptorPool, nullptr);
	}

	bool VulkanDescriptorPool::allocate_descriptor(const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptorSet) const
	{
		const auto device = Application::get().get_context().get_logical_device();

		auto allocationInfo = VkDescriptorSetAllocateInfo();
		allocationInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocationInfo.descriptorPool = m_descriptorPool;
		allocationInfo.pSetLayouts = &descriptorSetLayout;
		allocationInfo.descriptorSetCount = 1;

		const auto result = vkAllocateDescriptorSets(device, &allocationInfo, &descriptorSet);
		VULKAN_CHECK(result);

		return result == VK_SUCCESS;
	}

	void VulkanDescriptorPool::free_descriptors(const std::vector<VkDescriptorSet>& descriptors) const
	{
		const auto device = Application::get().get_context().get_logical_device();

		vkFreeDescriptorSets(device, m_descriptorPool, static_cast<uint32_t>(descriptors.size()), descriptors.data());
	}

	void VulkanDescriptorPool::reset_pool() const
	{
		const auto device = Application::get().get_context().get_logical_device();

		vkResetDescriptorPool(device, m_descriptorPool, 0);
	}

	//
	// VulkanDescriptorSetLayout::Builder
	//

	VulkanDescriptorSetLayout::Builder& VulkanDescriptorSetLayout::Builder::add_binding(const uint32_t binding, const VkDescriptorType descriptorType, const VkShaderStageFlags stageFlags, const uint32_t count)
	{
		LOG_ASSERT((m_bindings.contains(binding) == false), "Binding already exists");

		auto layoutBinding = VkDescriptorSetLayoutBinding();
		layoutBinding.binding = binding;
		layoutBinding.descriptorType = descriptorType;
		layoutBinding.descriptorCount = count;
		layoutBinding.stageFlags = stageFlags;

		m_bindings[binding] = layoutBinding;

		return *this;
	}

	std::unique_ptr<VulkanDescriptorSetLayout> VulkanDescriptorSetLayout::Builder::build() const
	{
		return std::make_unique<VulkanDescriptorSetLayout>(m_bindings);
	}

	//
	// VulkanDescriptorSetLayout
	//

	VulkanDescriptorSetLayout::VulkanDescriptorSetLayout(const std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding>& bindings)
	{
		const auto device = Application::get().get_context().get_logical_device();

		m_bindings = bindings;
		auto setLayoutBindings = std::vector<VkDescriptorSetLayoutBinding>();
		for (const auto& binding : std::views::values(bindings))
		{
			setLayoutBindings.push_back(binding);
		}

		auto descriptorSetLayoutInfo = VkDescriptorSetLayoutCreateInfo();
		descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		descriptorSetLayoutInfo.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
		descriptorSetLayoutInfo.pBindings = setLayoutBindings.data();

		const auto result = vkCreateDescriptorSetLayout(device, &descriptorSetLayoutInfo, nullptr, &m_descriptorSetLayout);
		VULKAN_CHECK(result);
	}

	VulkanDescriptorSetLayout::~VulkanDescriptorSetLayout()
	{
		const auto device = Application::get().get_context().get_logical_device();

		vkDestroyDescriptorSetLayout(device, m_descriptorSetLayout, nullptr);
	}

	DescriptorWriter& DescriptorWriter::write_buffer(const uint32_t binding, const VkDescriptorBufferInfo& bufferInfo)
	{
		LOG_ASSERT((m_setLayout.m_bindings.contains(binding) == true), "Binding does not exists");

		const auto& description = m_setLayout.m_bindings[binding];

		LOG_ASSERT((description.descriptorCount == 1), "Multiple bindings is not supported");

		auto write = VkWriteDescriptorSet();
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.descriptorType = description.descriptorType;
		write.dstBinding = binding;
		write.pBufferInfo = &bufferInfo;
		write.descriptorCount = 1;
		m_descriptorSets.push_back(write);

		return *this;
	}

	DescriptorWriter &DescriptorWriter::write_image(const uint32_t binding, const VkDescriptorImageInfo& imageInfo)
	{
		LOG_ASSERT((m_setLayout.m_bindings.contains(binding) == true), "Binding does not exists");

		const auto& description = m_setLayout.m_bindings[binding];

		LOG_ASSERT((description.descriptorCount == 1), "Multiple bindings is not supported");

		auto write = VkWriteDescriptorSet();
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.descriptorType = description.descriptorType;
		write.dstBinding = binding;
		write.pImageInfo = &imageInfo;
		write.descriptorCount = 1;
		m_descriptorSets.push_back(write);

		return *this;
	}

	bool DescriptorWriter::build(VkDescriptorSet& set)
	{
		const bool success = m_pool.allocate_descriptor(m_setLayout.get_descriptor_set_layout(), set);
		if (success == false)
		{
			return false;
		}

		overwrite(set);
		return true;
	}

	void DescriptorWriter::overwrite(const VkDescriptorSet& set)
	{
		const auto device = Application::get().get_context().get_logical_device();

		for (auto &descriptorSet : m_descriptorSets)
		{
			descriptorSet.dstSet = set;
		}

		vkUpdateDescriptorSets(device, m_descriptorSets.size(), m_descriptorSets.data(), 0, nullptr);
	}
}
