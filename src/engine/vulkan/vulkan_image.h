#pragma once

#include <vk_mem_alloc.h>

namespace SDLarria 
{
	class VulkanImage 
	{
	public:
		VulkanImage() = default;
		VulkanImage(VmaAllocator allocator, const VkExtent2D& size);
		~VulkanImage() = default;

		void CopyInto(const VulkanImage& target) const;
		void CopyRaw(VkImage target, const VkExtent2D& imageSize) const;
		void Transit(VkImageLayout newLayout);
		static void Transit(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout);

		const VkFormat* GetImageFormat() const { return &m_ImageFormat; }
		const VkImage& GetRawImage() const { return m_Image; }
		const VkImageView& GetImageView() const { return m_ImageView; }
		VkExtent3D GetImageSize() const { return m_ImageExtent; }
		VmaAllocation GetAllocation() const { return m_Allocation; }

	private:
		VkImageLayout m_Layout = VK_IMAGE_LAYOUT_UNDEFINED;

		VkImage m_Image = nullptr;
		VkImageView m_ImageView = nullptr;
		VkFormat m_ImageFormat;
		VkExtent3D m_ImageExtent;
		VmaAllocation m_Allocation = nullptr;
	};
}