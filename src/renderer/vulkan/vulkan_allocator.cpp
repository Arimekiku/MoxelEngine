#define VMA_IMPLEMENTATION
#include "vulkan_allocator.h"
#include "vulkan.h"
#include "renderer/application.h"

namespace Moxel
{
	VmaAllocator VulkanAllocator::m_allocator;
	std::unordered_map<UUID, VmaAllocation> VulkanAllocator::m_allocatorAssets;

	void VulkanAllocator::initialize()
	{
		const auto& instance = Application::get().get_context();

		auto allocatorInfo = VmaAllocatorCreateInfo();
		allocatorInfo.physicalDevice = instance.get_physical_device();
		allocatorInfo.device = instance.get_logical_device();
		allocatorInfo.instance = instance.get_instance();
		allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
		vmaCreateAllocator(&allocatorInfo, &m_allocator);
	}

	VulkanBuffer VulkanAllocator::allocate_buffer(const VkBufferCreateInfo& bufferCreateInfo, const VmaMemoryUsage usage)
	{
		auto buffer = VulkanBuffer();

		auto allocCreateInfo = VmaAllocationCreateInfo();
		allocCreateInfo.usage = usage;
		allocCreateInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

		//TODO: fix bad allocation situation
		//VmaAllocation allocation;
		//m_allocatorAssets.emplace(buffer.get_uuid(), allocation);

		vmaCreateBuffer(m_allocator, &bufferCreateInfo, &allocCreateInfo, &buffer.Buffer, &buffer.Allocation, &buffer.AllocationInfo);

		return buffer;
	}

	VulkanImage VulkanAllocator::allocate_image(const VulkanImageSpecs& specs, const VmaMemoryUsage usage)
	{
		const auto device = Application::get().get_context().get_logical_device();

		// image creation process
		auto image = VulkanImage();

		auto allocationInfo = VmaAllocationCreateInfo();
		allocationInfo.usage = usage;
		allocationInfo.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

		VmaAllocation allocation;
		m_allocatorAssets.emplace(image.get_uuid(), allocation);

		// allocation
		image.ImageFormat = specs.Format;
		image.ImageExtent =
		{
			specs.InitialSize.width,
			specs.InitialSize.height,
			1
		};

		auto imageInfo = VkImageCreateInfo();
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.pNext = nullptr;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.format = image.ImageFormat;
		imageInfo.extent = image.ImageExtent;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.usage = specs.ImageUsages;

		vmaCreateImage(m_allocator, &imageInfo, &allocationInfo, &image.Image, &allocation, nullptr);

		auto imageViewInfo = VkImageViewCreateInfo();
		imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewInfo.pNext = nullptr;
		imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewInfo.image = image.Image;
		imageViewInfo.format = image.ImageFormat;
		imageViewInfo.subresourceRange.baseMipLevel = 0;
		imageViewInfo.subresourceRange.levelCount = 1;
		imageViewInfo.subresourceRange.baseArrayLayer = 0;
		imageViewInfo.subresourceRange.layerCount = 1;
		imageViewInfo.subresourceRange.aspectMask = specs.ImageAspects;

		const auto result = vkCreateImageView(device, &imageViewInfo, nullptr, &image.ImageView);
		VULKAN_CHECK(result);

		return image;
	}

	void VulkanAllocator::destroy()
	{
		vmaDestroyAllocator(m_allocator);
	}

	void VulkanAllocator::destroy_vulkan_image(const VulkanImage& image)
	{
		const auto device = Application::get().get_context().get_logical_device();
		const auto allocation = m_allocatorAssets.at(image.get_uuid());

		vkDestroyImageView(device, image.ImageView, nullptr);
		vmaDestroyImage(m_allocator, image.Image, allocation);
	}

	void VulkanAllocator::destroy_buffer(const VulkanBuffer& buffer)
	{
		vmaDestroyBuffer(m_allocator, buffer.Buffer, buffer.Allocation);
	}
}