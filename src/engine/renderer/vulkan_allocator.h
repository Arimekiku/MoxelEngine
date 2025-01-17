#pragma once

#include "vulkan_buffer.h"
#include "vulkan_image.h"

#include <unordered_map>

namespace Moxel
{
	class VulkanAllocator
	{
	public:
		VulkanAllocator() = default;

		void initialize();
		void destroy() const;

		BufferAsset allocate_buffer(const VkBufferCreateInfo& bufferCreateInfo, VmaMemoryUsage usage);
		void destroy_buffer(const BufferAsset& buffer);

		ImageAsset allocate_image(const VkImageCreateInfo& imageCreateInfo, VmaMemoryUsage usage);
		void destroy_image(const ImageAsset& image);
	private:
		VmaAllocator m_allocator;

		std::unordered_map<UUID, VmaAllocation> m_allocatorAssets;
	};
}
