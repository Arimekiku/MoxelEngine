#include "vulkan_allocator.h"
#include "vulkan.h"

#include <vector>

namespace SDLarria 
{
    void DescriptorAllocator::Initialize(VulkanInstance& instance, uint32_t maxSets, std::span<PoolSizeRatio> poolRatios)
    {
        std::vector<VkDescriptorPoolSize> poolSizes;
        for (PoolSizeRatio ratio : poolRatios) 
        {
            auto poolSize = VkDescriptorPoolSize();
            poolSize.type = ratio.Type;
            poolSize.descriptorCount = uint32_t(ratio.Ratio * maxSets);

            poolSizes.push_back(poolSize);
        }

        auto pool_info = VkDescriptorPoolCreateInfo();
        pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.flags = 0;
        pool_info.maxSets = maxSets;
        pool_info.poolSizeCount = (uint32_t)poolSizes.size();
        pool_info.pPoolSizes = poolSizes.data();

        m_Device = instance.GetLogicalDevice();
        vkCreateDescriptorPool(m_Device, &pool_info, nullptr, &m_Pool);
    }

    void DescriptorAllocator::Destroy() const
    {
        vkDestroyDescriptorPool(m_Device, m_Pool, nullptr);
    }

    void DescriptorAllocator::ClearDescriptors() const 
    {
        vkResetDescriptorPool(m_Device, m_Pool, 0);
    }

    VkDescriptorSet DescriptorAllocator::AllocateSet(VkDescriptorSetLayout layout) const 
    {
        auto allocInfo = VkDescriptorSetAllocateInfo();
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.pNext = nullptr;
        allocInfo.descriptorPool = m_Pool;
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = &layout;

        auto set = VkDescriptorSet();
        auto result = vkAllocateDescriptorSets(m_Device, &allocInfo, &set);
        VULKAN_CHECK(result);

        return set;
    }

	void BufferAllocator::Initialize(VulkanInstance& instance) 
	{
        auto allocatorInfo = VmaAllocatorCreateInfo();
        allocatorInfo.physicalDevice = instance.GetPhysicalDevice();
        allocatorInfo.device = instance.GetLogicalDevice();
        allocatorInfo.instance = instance.GetInstance();
        allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
        vmaCreateAllocator(&allocatorInfo, &m_Allocator);

        m_Device = instance.GetLogicalDevice();
	}

    void BufferAllocator::Destroy() const
    {
        vmaDestroyAllocator(m_Allocator);
    }

    void BufferAllocator::DestroyVulkanImage(VulkanImage& image) const
    {
        vkDestroyImageView(m_Device, image.GetImageView(), nullptr);
        vmaDestroyImage(m_Allocator, image.GetRawImage(), image.GetAllocation());
    }
}