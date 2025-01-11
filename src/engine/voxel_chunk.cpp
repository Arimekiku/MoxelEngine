#include "voxel_chunk.h"
#include "PerlinNoise.hpp"
#include "render_quad.h"

namespace Moxel
{
	static int s_seed = 123456u;
	static int s_chunkSize = 16;

	VoxelChunk::VoxelChunk(const glm::vec3 position)
	{
		m_Position = position;
	}

	VoxelChunk::~VoxelChunk()
	{
		m_VertexArray = nullptr;
	}

	void VoxelChunk::RecreateChunk()
	{
		siv::PerlinNoise::seed_type pseed = s_seed;
		const auto perlin = siv::PerlinNoise(pseed);

		for (int z = 0; z < s_chunkSize; ++z)
		{
			for (int y = 0; y < s_chunkSize; ++y)
			{
				for (int x = 0; x < s_chunkSize; ++x)
				{
					const auto offset = glm::vec3(m_Position.x * s_chunkSize + x, m_Position.y * s_chunkSize + y, m_Position.z * s_chunkSize + z);
					const double noise = perlin.octave3D_01(offset.x * 0.01f, offset.y * 0.01f, offset.z * 0.01f, 4);

					m_Blocks[x][y][z] = noise > 0.5;
				}
			}
		}

		auto totalVertices = std::vector<Vertex>();
		auto totalIndices = std::vector<uint32_t>();
		int indexOffset = 0;
		for (int z = 0; z < s_chunkSize; ++z)
		{
			for (int y = 0; y < s_chunkSize; ++y)
			{
				for (int x = 0; x < s_chunkSize; ++x)
				{
					if (m_Blocks[x][y][z] == false)
					{
						continue;
					}

					const auto positionOffset = m_Position * (float)s_chunkSize + glm::vec3(x, y, z);
					auto indexQuadOffset = 0;

					if (y == s_chunkSize - 1 || m_Blocks[x][y + 1][z] == false)
					{
						auto quadUp = RenderQuad(Direction::Up, positionOffset);
						quadUp.AddIndicesOffset(indexOffset + indexQuadOffset);
						totalVertices.insert(totalVertices.end(), quadUp.GetVertices().begin(), quadUp.GetVertices().end());
						totalIndices.insert(totalIndices.end(), quadUp.GetIndices().begin(), quadUp.GetIndices().end());

						indexQuadOffset += 4;
					}

					if (y == 0 || m_Blocks[x][y - 1][z] == false)
					{
						auto quadDown = RenderQuad(Direction::Down, positionOffset);
						quadDown.AddIndicesOffset(indexOffset + indexQuadOffset);
						totalVertices.insert(totalVertices.end(), quadDown.GetVertices().begin(), quadDown.GetVertices().end());
						totalIndices.insert(totalIndices.end(), quadDown.GetIndices().begin(), quadDown.GetIndices().end());

						indexQuadOffset += 4;
					}

					if (x == s_chunkSize - 1 || m_Blocks[x + 1][y][z] == false)
					{
						auto quadRight = RenderQuad(Direction::Right, positionOffset);
						quadRight.AddIndicesOffset(indexOffset + indexQuadOffset);
						totalVertices.insert(totalVertices.end(), quadRight.GetVertices().begin(), quadRight.GetVertices().end());
						totalIndices.insert(totalIndices.end(), quadRight.GetIndices().begin(), quadRight.GetIndices().end());

						indexQuadOffset += 4;
					}

					if (x == 0 || m_Blocks[x - 1][y][z] == false)
					{
						auto quadLeft = RenderQuad(Direction::Left, positionOffset);
						quadLeft.AddIndicesOffset(indexOffset + indexQuadOffset);
						totalVertices.insert(totalVertices.end(), quadLeft.GetVertices().begin(), quadLeft.GetVertices().end());
						totalIndices.insert(totalIndices.end(), quadLeft.GetIndices().begin(), quadLeft.GetIndices().end());

						indexQuadOffset += 4;
					}

					if (z == 0 || m_Blocks[x][y][z - 1] == false)
					{
						auto quadBack = RenderQuad(Direction::Backward, positionOffset);
						quadBack.AddIndicesOffset(indexOffset + indexQuadOffset);
						totalVertices.insert(totalVertices.end(), quadBack.GetVertices().begin(), quadBack.GetVertices().end());
						totalIndices.insert(totalIndices.end(), quadBack.GetIndices().begin(), quadBack.GetIndices().end());

						indexQuadOffset += 4;
					}

					if (z == s_chunkSize - 1 || m_Blocks[x][y][z + 1] == false)
					{
						auto quadForw = RenderQuad(Direction::Forward, positionOffset);
						quadForw.AddIndicesOffset(indexOffset + indexQuadOffset);
						totalVertices.insert(totalVertices.end(), quadForw.GetVertices().begin(), quadForw.GetVertices().end());
						totalIndices.insert(totalIndices.end(), quadForw.GetIndices().begin(), quadForw.GetIndices().end());

						indexQuadOffset += 4;
					}

					indexOffset += indexQuadOffset;
				}
			}
		}

		if (totalIndices.empty())
		{
			m_VertexArray = nullptr;
			return;
		}

		const auto vao = std::make_shared<VulkanVertexArray>(totalIndices, totalVertices);
		m_VertexArray = std::make_shared<RenderMesh>(vao);
	}
}
