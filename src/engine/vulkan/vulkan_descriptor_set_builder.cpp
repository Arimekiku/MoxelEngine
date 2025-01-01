#include "vulkan_descriptor_set_builder.h"
#include "vulkan.h"
#include "vulkan_renderer.h"

#include <ranges>

namespace SDLarria
{
	VulkanDescriptorPool::Builder &VulkanDescriptorPool::Builder::AddPoolSize(const VkDescriptorType descriptorType, const uint32_t count)
	{
		m_PoolSizes.push_back(VkDescriptorPoolSize(descriptorType, count));

		return *this;
	}

	VulkanDescriptorPool::Builder &VulkanDescriptorPool::Builder::SetCreateFlags(const VkDescriptorPoolCreateFlags flags)
	{
		m_PoolFlags = flags;

		return *this;
	}

	VulkanDescriptorPool::Builder &VulkanDescriptorPool::Builder::SetMaxSets(const uint32_t count)
	{
		m_MaxSets = count;

		return *this;
	}

	std::unique_ptr<VulkanDescriptorPool> VulkanDescriptorPool::Builder::Build() const
	{
		return std::make_unique<VulkanDescriptorPool>(m_MaxSets, m_PoolFlags, m_PoolSizes);
	}

	VulkanDescriptorPool::VulkanDescriptorPool(const uint32_t maxSets, const VkDescriptorPoolCreateFlags poolFlags, const std::vector<VkDescriptorPoolSize> &poolSizes)
	{
		const auto device = VulkanRenderer::Get().GetContext().GetLogicalDevice();

		auto descriptorPoolInfo = VkDescriptorPoolCreateInfo();
		descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		descriptorPoolInfo.pPoolSizes = poolSizes.data();
		descriptorPoolInfo.maxSets = maxSets;
		descriptorPoolInfo.flags = poolFlags;

		const auto result = vkCreateDescriptorPool(device, &descriptorPoolInfo, nullptr, &m_DescriptorPool);
		VULKAN_CHECK(result);
	}

	VulkanDescriptorPool::~VulkanDescriptorPool()
	{
		const auto device = VulkanRenderer::Get().GetContext().GetLogicalDevice();

		vkDestroyDescriptorPool(device, m_DescriptorPool, nullptr);
	}

	bool VulkanDescriptorPool::AllocateDescriptor(VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptorSet) const
	{
		const auto device = VulkanRenderer::Get().GetContext().GetLogicalDevice();

		auto allocationInfo = VkDescriptorSetAllocateInfo();
		allocationInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocationInfo.descriptorPool = m_DescriptorPool;
		allocationInfo.pSetLayouts = &descriptorSetLayout;
		allocationInfo.descriptorSetCount = 1;

		const auto result = vkAllocateDescriptorSets(device, &allocationInfo, &descriptorSet);
		VULKAN_CHECK(result);

		return result == VK_SUCCESS;
	}

	void VulkanDescriptorPool::FreeDescriptors(const std::vector<VkDescriptorSet>& descriptors) const
	{
		const auto device = VulkanRenderer::Get().GetContext().GetLogicalDevice();

		vkFreeDescriptorSets(device, m_DescriptorPool, static_cast<uint32_t>(descriptors.size()), descriptors.data());
	}

	void VulkanDescriptorPool::ResetPool() const
	{
		const auto device = VulkanRenderer::Get().GetContext().GetLogicalDevice();

		vkResetDescriptorPool(device, m_DescriptorPool, 0);
	}

	VulkanDescriptorSetLayout::Builder& VulkanDescriptorSetLayout::Builder::AddBinding(const uint32_t binding, const VkDescriptorType descriptorType, const VkShaderStageFlags stageFlags, const uint32_t count)
	{
		LOG_ASSERT(m_Bindings.contains(binding) == false, "Binding already exists");

		auto layoutBinding = VkDescriptorSetLayoutBinding();
		layoutBinding.binding = binding;
		layoutBinding.descriptorType = descriptorType;
		layoutBinding.descriptorCount = count;
		layoutBinding.stageFlags = stageFlags;

		m_Bindings[binding] = layoutBinding;

		return *this;
	}

	std::unique_ptr<VulkanDescriptorSetLayout> VulkanDescriptorSetLayout::Builder::Build() const
	{
		return std::make_unique<VulkanDescriptorSetLayout>(m_Bindings);
	}

	VulkanDescriptorSetLayout::VulkanDescriptorSetLayout(const std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding>& bindings)
	{
		const auto device = VulkanRenderer::Get().GetContext().GetLogicalDevice();

		m_Bindings = bindings;
		auto setLayoutBindings = std::vector<VkDescriptorSetLayoutBinding>();
		for (const auto& binding : std::views::values(bindings))
		{
			setLayoutBindings.push_back(binding);
		}

		auto descriptorSetLayoutInfo = VkDescriptorSetLayoutCreateInfo();
		descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		descriptorSetLayoutInfo.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
		descriptorSetLayoutInfo.pBindings = setLayoutBindings.data();

		const auto result = vkCreateDescriptorSetLayout(device, &descriptorSetLayoutInfo, nullptr, &m_DescriptorSetLayout);
		VULKAN_CHECK(result);
	}

	VulkanDescriptorSetLayout::~VulkanDescriptorSetLayout()
	{
		const auto device = VulkanRenderer::Get().GetContext().GetLogicalDevice();

		vkDestroyDescriptorSetLayout(device, m_DescriptorSetLayout, nullptr);
	}

	DescriptorWriter& DescriptorWriter::WriteBuffer(const uint32_t binding, const VkDescriptorBufferInfo& bufferInfo)
	{
		LOG_ASSERT(m_SetLayout.m_Bindings.contains(binding) == true, "Binding does not exists");

		const auto& description = m_SetLayout.m_Bindings[binding];

		LOG_ASSERT(description.descriptorCount == 1, "Multiple bindings is not supported");

		auto write = VkWriteDescriptorSet();
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.descriptorType = description.descriptorType;
		write.dstBinding = binding;
		write.pBufferInfo = &bufferInfo;
		write.descriptorCount = 1;
		m_DescriptorSets.push_back(write);

		return *this;
	}

	DescriptorWriter &DescriptorWriter::WriteImage(const uint32_t binding, const VkDescriptorImageInfo& imageInfo)
	{
		LOG_ASSERT(m_SetLayout.m_Bindings.contains(binding) == true, "Binding does not exists");

		const auto& description = m_SetLayout.m_Bindings[binding];

		LOG_ASSERT(description.descriptorCount == 1, "Multiple bindings is not supported");

		auto write = VkWriteDescriptorSet();
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.descriptorType = description.descriptorType;
		write.dstBinding = binding;
		write.pImageInfo = &imageInfo;
		write.descriptorCount = 1;
		m_DescriptorSets.push_back(write);

		return *this;
	}

	bool DescriptorWriter::Build(VkDescriptorSet& set)
	{
		bool success = m_Pool.AllocateDescriptor(m_SetLayout.GetDescriptorSetLayout(), set);
		if (success == false)
		{
			return false;
		}

		Overwrite(set);
		return true;
	}

	void DescriptorWriter::Overwrite(const VkDescriptorSet& set)
	{
		const auto device = VulkanRenderer::Get().GetContext().GetLogicalDevice();

		for (auto &descriptorSet : m_DescriptorSets)
		{
			descriptorSet.dstSet = set;
		}

		vkUpdateDescriptorSets(device, m_DescriptorSets.size(), m_DescriptorSets.data(), 0, nullptr);
	}
}
