#define VMA_IMPLEMENTATION
#include "vulkan_allocator.h"
#include "renderer/application.h"

namespace Moxel
{
	VmaAllocator VulkanAllocator::m_Allocator;

	void VulkanAllocator::Initialize()
	{
		const auto& instance = Application::Get().GetContext();

		auto allocatorInfo = VmaAllocatorCreateInfo();
		allocatorInfo.physicalDevice = instance.GetPhysicalDevice();
		allocatorInfo.device = instance.GetLogicalDevice();
		allocatorInfo.instance = instance.GetInstance();
		allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
		vmaCreateAllocator(&allocatorInfo, &m_Allocator);
	}

	VulkanBuffer VulkanAllocator::AllocateBuffer(const VkBufferCreateInfo& bufferCreateInfo, const VmaMemoryUsage usage)
	{
		auto buffer = VulkanBuffer();

		auto allocCreateInfo = VmaAllocationCreateInfo();
		allocCreateInfo.usage = usage;
		allocCreateInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

		vmaCreateBuffer(m_Allocator, &bufferCreateInfo, &allocCreateInfo, &buffer.Buffer, &buffer.Allocation, &buffer.AllocationInfo);

		return buffer;
	}

	void VulkanAllocator::AllocateImage(const VkImageCreateInfo& imageCreateInfo, const VmaMemoryUsage usage, VkImage& outImage, VmaAllocation& outAllocation)
	{
		auto allocationInfo = VmaAllocationCreateInfo();
		allocationInfo.usage = usage;
		allocationInfo.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

		vmaCreateImage(m_Allocator, &imageCreateInfo, &allocationInfo, &outImage, &outAllocation, nullptr);
	}

	void* VulkanAllocator::MapData(VmaAllocation allocation)
	{
		void* mappedMemory = nullptr;
		vmaMapMemory(m_Allocator, allocation, &mappedMemory);
		return mappedMemory;
	}

	void VulkanAllocator::UnmapData(VmaAllocation allocation)
	{
		vmaUnmapMemory(m_Allocator, allocation);
	}

	void VulkanAllocator::Destroy()
	{
		vmaDestroyAllocator(m_Allocator);
	}

	void VulkanAllocator::DestroyVulkanImage(const VulkanImage& image)
	{
		const auto device = Application::Get().GetContext().GetLogicalDevice();

		vkDestroyImageView(device, image.GetImageView(), nullptr);
		vmaDestroyImage(m_Allocator, image.GetRawImage(), image.GetAllocation());
	}

	void VulkanAllocator::DestroyBuffer(const VulkanBuffer& buffer)
	{
		vmaDestroyBuffer(m_Allocator, buffer.Buffer, buffer.Allocation);
	}
}
