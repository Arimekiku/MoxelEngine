#pragma once

#include <memory>
#include <span>

#include "vulkan_vertex_array.h"
#include "vulkan_image.h"

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

		void Initialize(uint32_t maxSets, std::span<PoolSizeRatio> poolRatios);
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

		void Initialize();
		void Destroy() const;

		VmaAllocator GetAllocator() const { return m_Allocator; }

		void DestroyVulkanImage(const std::shared_ptr<VulkanImage>& image) const;
		void DestroyBuffer(const BufferArray& buffer) const;

	private:
		VkDevice m_Device;
		VmaAllocator m_Allocator;
	};
}
