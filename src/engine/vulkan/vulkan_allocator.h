#pragma once

#include "vulkan_instance.h"
#include "vulkan_image.h"

#include <span>

namespace SDLarria 
{
	struct PoolSizeRatio
	{
		VkDescriptorType Type;
		float Ratio;
	};

	class DescriptorAllocator
	{
	public:
		DescriptorAllocator() = default;

		void Initialize(VulkanInstance& instance, uint32_t maxSets, std::span<PoolSizeRatio> poolRatios);
		void Destroy() const;

		VkDescriptorSet AllocateSet(VkDescriptorSetLayout layout) const;
		void ClearDescriptors() const;

	private:
		VkDevice m_Device;
		VkDescriptorPool m_Pool;
	};

	class BufferAllocator 
	{
	public:
		BufferAllocator() = default;

		void Initialize(VulkanInstance& instance);
		void Destroy() const;

		VmaAllocator GetAllocator() const { return m_Allocator; }

		void DestroyVulkanImage(VulkanImage& image) const;

	private:
		VkDevice m_Device;
		VmaAllocator m_Allocator;
	};
}