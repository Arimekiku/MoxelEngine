#include "vulkan_allocator.h"

namespace SDLarria 
{
	void VulkanAllocator::Initialize(VulkanInstance& instance) 
	{
        auto allocatorInfo = VmaAllocatorCreateInfo();
        allocatorInfo.physicalDevice = instance.GetPhysicalDevice();
        allocatorInfo.device = instance.GetLogicalDevice();
        allocatorInfo.instance = instance.GetInstance();
        allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
        vmaCreateAllocator(&allocatorInfo, &m_Allocator);

        m_Device = instance.GetLogicalDevice();
	}

    void VulkanAllocator::Destroy() const
    {
        vmaDestroyAllocator(m_Allocator);
    }

    void VulkanAllocator::DestroyVulkanImage(VulkanImage& image) const
    {
        vkDestroyImageView(m_Device, image.GetImageView(), nullptr);
        vmaDestroyImage(m_Allocator, image.GetRawImage(), image.GetAllocation());
    }
}