#pragma once

#include "engine/core/asset.h"
#include "scene/voxels/render_quad.h"

#include <vk_mem_alloc.h>

namespace Moxel
{
	struct BufferAsset final : Asset
	{
		VkBuffer Buffer = nullptr;
		VmaAllocationInfo AllocationInfo = VmaAllocationInfo();
	};

	class VulkanVertexArray
	{
	public:
		VulkanVertexArray() = default;
		VulkanVertexArray(const std::vector<uint32_t>& indices, const std::vector<VoxelVertex>& vertices);

		const std::vector<VoxelVertex>& get_vertices() { return m_vertices; }
		size_t get_index_buffer_size() const { return m_indices; }

		BufferAsset& get_vertex_buffer() { return m_vertexBuffer; }
		BufferAsset& get_index_buffer() { return m_indexBuffer; }
	private:
		std::vector<VoxelVertex> m_vertices;
		size_t m_indices = 0;

		BufferAsset m_vertexBuffer;
		BufferAsset m_indexBuffer;
	};

	class VulkanBufferUniform
	{
	public:
		VulkanBufferUniform(uint32_t bufferSize);
		~VulkanBufferUniform();

		const VkDescriptorBufferInfo& get_descriptor_info() const { return m_descriptorInfo; }

		void write_data(const void* data, uint32_t size) const;
	private:
		uint32_t m_size = 0;

		BufferAsset m_buffer;
		VkDescriptorBufferInfo m_descriptorInfo;

		void* m_data = nullptr;
	};
}
