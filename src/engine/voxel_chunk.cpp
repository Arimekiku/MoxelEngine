#include "voxel_chunk.h"

namespace Moxel
{
	VoxelChunk::VoxelChunk()
	{
		for (auto &m_BlockPlane : m_Blocks)
		{
			for (auto &m_BlockLine : m_BlockPlane)
			{
				for (bool &m_Block : m_BlockLine)
				{
					m_Block = true;
				}
			}
		}

		auto totalVertices = std::vector<Vertex>();
		auto totalIndices = std::vector<uint32_t>();
		int indexOffset = 0;
		for (int z = 0; z < 10; ++z)
		{
			for (int y = 0; y < 10; ++y)
			{
				for (int x = 0; x < 10; ++x)
				{
					if (m_Blocks[x][y][z] == false)
					{
						continue;
					}

					const auto positionOffset = glm::vec3(x, y, z);

					auto vertices = std::vector<Vertex>(8);
					vertices[0].Position = glm::vec3(-0.5f, -0.5f, 0.5f) + positionOffset;
					vertices[1].Position = glm::vec3( 0.5f, -0.5f, 0.5f) + positionOffset;
					vertices[2].Position = glm::vec3( 0.5f,  0.5f, 0.5f) + positionOffset;
					vertices[3].Position = glm::vec3(-0.5f,  0.5f, 0.5f) + positionOffset;
					vertices[4].Position = glm::vec3(-0.5f, -0.5f, -0.5f) + positionOffset;
					vertices[5].Position = glm::vec3( 0.5f, -0.5f, -0.5f) + positionOffset;
					vertices[6].Position = glm::vec3( 0.5f,  0.5f, -0.5f) + positionOffset;
					vertices[7].Position = glm::vec3(-0.5f,  0.5f, -0.5f) + positionOffset;

					vertices[0].Color = glm::vec3(0, 0, 1);
					vertices[1].Color = glm::vec3(0, 1, 0);
					vertices[2].Color = glm::vec3(0, 1, 1);
					vertices[3].Color = glm::vec3(1, 0, 0);
					vertices[4].Color = glm::vec3(1, 0, 1);
					vertices[5].Color = glm::vec3(1, 1, 0);
					vertices[6].Color = glm::vec3(1, 1, 1);
					vertices[7].Color = glm::vec3(0, 0, 0);
					totalVertices.insert(totalVertices.end(), vertices.begin(), vertices.end());

					std::vector<uint32_t> indices =
					{
						0, 1, 2, 2, 3, 0,
						1, 5, 6, 6, 2, 1,
						7, 6, 5, 5, 4, 7,
						4, 0, 3, 3, 7, 4,
						4, 5, 1, 1, 0, 4,
						3, 2, 6, 6, 7, 3,
					};

					for (uint32_t& index : indices)
					{
						index += indexOffset;
					}
					totalIndices.insert(totalIndices.end(), indices.begin(), indices.end());

					indexOffset += 8;
				}
			}
		}

		const auto vao = std::make_shared<VulkanVertexArray>(totalIndices, totalVertices);
		m_VertexArray = std::make_shared<RenderMesh>(vao);
	}
}