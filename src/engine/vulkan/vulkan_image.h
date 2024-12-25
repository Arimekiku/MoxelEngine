#pragma once

#include <vk_mem_alloc.h>

namespace SDLarria 
{
	class VulkanImage 
	{
	public:
		VulkanImage() = default;
		VulkanImage(VkDevice device, VmaAllocator allocator, VkExtent2D& size);
		~VulkanImage() = default;

		void Copy(VkCommandBuffer cmd, VulkanImage& target) const;
		void Copy(VkCommandBuffer cmd, VkImage target, VkExtent2D imageSize) const;
		void Transit(VkCommandBuffer cmd, VkImageLayout newLayout);
		static void Transit(VkCommandBuffer cmd, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout);

		VkImage GetRawImage() const { return m_Image; }
		VkImageView GetImageView() const { return m_ImageView; }
		VmaAllocation GetAllocation() const { return m_Allocation; }

	private:
		VkImageLayout m_Layout = VK_IMAGE_LAYOUT_UNDEFINED;

		VkImage m_Image = nullptr;
		VkImageView m_ImageView = nullptr;
		VkFormat m_ImageFormat;
		VkExtent3D m_ImageExtent = VkExtent3D();
		VmaAllocation m_Allocation = nullptr;
	};
}