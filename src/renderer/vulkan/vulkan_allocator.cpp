#define VMA_IMPLEMENTATION
#include "vulkan_allocator.h"
#include "vulkan.h"
#include "renderer/application.h"

namespace Moxel
{
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

	void VulkanAllocator::destroy() const
	{
		vmaDestroyAllocator(m_allocator);
	}

	BufferAsset VulkanAllocator::allocate_buffer(const VkBufferCreateInfo& bufferCreateInfo, const VmaMemoryUsage usage)
	{
		auto buffer = BufferAsset();

		auto allocCreateInfo = VmaAllocationCreateInfo();
		allocCreateInfo.usage = usage;
		allocCreateInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

		VmaAllocation allocation;
		vmaCreateBuffer(m_allocator, &bufferCreateInfo, &allocCreateInfo, &buffer.Buffer, &allocation, &buffer.AllocationInfo);
		m_allocatorAssets[buffer.get_uuid()] = allocation;

		return buffer;
	}

	void VulkanAllocator::destroy_buffer(const BufferAsset& buffer)
	{
		const auto bufferId = buffer.get_uuid();

		vmaDestroyBuffer(m_allocator, buffer.Buffer, m_allocatorAssets[bufferId]);
		m_allocatorAssets.erase(bufferId);
	}

	ImageAsset VulkanAllocator::allocate_image(const VkImageCreateInfo& imageCreateInfo, const VmaMemoryUsage usage)
	{
		auto image = ImageAsset();
		image.ImageFormat = imageCreateInfo.format;
		image.ImageExtent = imageCreateInfo.extent;
		image.Layout = imageCreateInfo.initialLayout;

		auto allocationInfo = VmaAllocationCreateInfo();
		allocationInfo.usage = usage;
		allocationInfo.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

		VmaAllocation allocation;
		vmaCreateImage(m_allocator, &imageCreateInfo, &allocationInfo, &image.Image, &allocation, nullptr);
		m_allocatorAssets[image.get_uuid()] = allocation;

		return image;
	}

	void VulkanAllocator::destroy_image(const ImageAsset& image)
	{
		const auto imageId = image.get_uuid();

		const auto device = Application::get().get_context().get_logical_device();
		vkDestroyImageView(device, image.ImageView, nullptr);

		vmaDestroyImage(m_allocator, image.Image, m_allocatorAssets[imageId]);
		m_allocatorAssets.erase(imageId);
	}
}