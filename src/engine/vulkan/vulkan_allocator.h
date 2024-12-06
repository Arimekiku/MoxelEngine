#pragma once

#include "vulkan_instance.h"
#include "vulkan_image.h"

namespace SDLarria 
{
	class VulkanAllocator 
	{
	public:
		VulkanAllocator() = default;

		void Initialize(VulkanInstance& instance);
		void Destroy() const;

		VmaAllocator GetAllocator() const { return m_Allocator; }

		void DestroyVulkanImage(VulkanImage& image) const;

	private:
		VkDevice m_Device;
		VmaAllocator m_Allocator;
	};
}