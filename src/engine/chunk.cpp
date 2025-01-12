#include "chunk.h"
#include "render_quad.h"

#include <PerlinNoise.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

namespace Moxel
{
	Chunk::Chunk(const int size)
	{
		m_blocks.resize(size);
	}

	Chunk::~Chunk()
	{
		m_blocks.clear();
		destroy_mesh();
	}

	glm::mat4 Chunk::get_trs_matrix(const ChunkPosition position)
	{
		const glm::mat4 translation = translate(glm::mat4(1.0f), glm::vec3(position.x, position.y, position.z));
		const glm::mat4 rotation = toMat4(glm::quat(radians(glm::vec3(0))));
		const glm::mat4 scaling = scale(glm::mat4(1.0f), glm::vec3(1.0f));

		return translation * rotation * scaling;
	}

	void Chunk::generate_mesh(const ChunkPosition position)
	{
		const int chunkSize = cbrt(m_blocks.size());

		// generate chunk data from perlin
		constexpr siv::PerlinNoise::seed_type seed = 123456u;
		const auto perlin = siv::PerlinNoise(seed);

		for (int z = 0; z < chunkSize; ++z)
		{
			for (int y = 0; y < chunkSize; ++y)
			{
				for (int x = 0; x < chunkSize; ++x)
				{
					const auto offset = glm::vec3(position.x * chunkSize + x, position.y * chunkSize + y, position.z * chunkSize + z);
					const double noise = perlin.octave3D_01(offset.x * 0.01f, offset.y * 0.01f, offset.z * 0.01f, 4);

					if (noise > 0.5f)
					{
						set_block(z * chunkSize * chunkSize + y * chunkSize + x);
					}
				}
			}
		}

		// generate mesh data from chunk
		auto totalVertices = std::vector<Vertex>();
		auto totalIndices = std::vector<uint32_t>();
		int indexOffset = 0;

		const auto chunkPositionAsVec3 = glm::i32vec3(position.x, position.y, position.z);
		for (int z = 0; z < chunkSize; ++z)
		{
			for (int y = 0; y < chunkSize; ++y)
			{
				for (int x = 0; x < chunkSize; ++x)
				{
					if (get_block(z * chunkSize * chunkSize + y * chunkSize + x) == false)
					{
						continue;
					}

					const auto positionOffset = chunkPositionAsVec3 * chunkSize + glm::i32vec3(x, y, z);
					auto indexQuadOffset = 0;

					if (y == chunkSize - 1 || get_block(z * chunkSize * chunkSize + (y + 1) * chunkSize + x) == false)
					{
						auto quadUp = RenderQuad(Direction::Up, positionOffset);
						quadUp.AddIndicesOffset(indexOffset + indexQuadOffset);
						totalVertices.insert(totalVertices.end(), quadUp.GetVertices().begin(), quadUp.GetVertices().end());
						totalIndices.insert(totalIndices.end(), quadUp.GetIndices().begin(), quadUp.GetIndices().end());

						indexQuadOffset += 4;
					}

					if (y == 0 || get_block(z * chunkSize * chunkSize + (y - 1) * chunkSize + x) == false)
					{
						auto quadDown = RenderQuad(Direction::Down, positionOffset);
						quadDown.AddIndicesOffset(indexOffset + indexQuadOffset);
						totalVertices.insert(totalVertices.end(), quadDown.GetVertices().begin(), quadDown.GetVertices().end());
						totalIndices.insert(totalIndices.end(), quadDown.GetIndices().begin(), quadDown.GetIndices().end());

						indexQuadOffset += 4;
					}

					if (x == chunkSize - 1 || get_block(z * chunkSize * chunkSize + y * chunkSize + x + 1) == false)
					{
						auto quadRight = RenderQuad(Direction::Right, positionOffset);
						quadRight.AddIndicesOffset(indexOffset + indexQuadOffset);
						totalVertices.insert(totalVertices.end(), quadRight.GetVertices().begin(), quadRight.GetVertices().end());
						totalIndices.insert(totalIndices.end(), quadRight.GetIndices().begin(), quadRight.GetIndices().end());

						indexQuadOffset += 4;
					}

					if (x == 0 || get_block(z * chunkSize * chunkSize + y * chunkSize + x - 1) == false)
					{
						auto quadLeft = RenderQuad(Direction::Left, positionOffset);
						quadLeft.AddIndicesOffset(indexOffset + indexQuadOffset);
						totalVertices.insert(totalVertices.end(), quadLeft.GetVertices().begin(), quadLeft.GetVertices().end());
						totalIndices.insert(totalIndices.end(), quadLeft.GetIndices().begin(), quadLeft.GetIndices().end());

						indexQuadOffset += 4;
					}

					if (z == 0 || get_block((z - 1) * chunkSize * chunkSize + y * chunkSize + x) == false)
					{
						auto quadBack = RenderQuad(Direction::Backward, positionOffset);
						quadBack.AddIndicesOffset(indexOffset + indexQuadOffset);
						totalVertices.insert(totalVertices.end(), quadBack.GetVertices().begin(), quadBack.GetVertices().end());
						totalIndices.insert(totalIndices.end(), quadBack.GetIndices().begin(), quadBack.GetIndices().end());

						indexQuadOffset += 4;
					}

					if (z == chunkSize - 1 || get_block((z + 1) * chunkSize * chunkSize + y * chunkSize + x) == false)
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
			return;
		}

		m_chunkMesh = std::make_shared<VulkanVertexArray>(totalIndices, totalVertices);
	}

	void Chunk::destroy_mesh()
	{
		if (m_chunkMesh == nullptr)
		{
			return;
		}

		m_chunkMesh = nullptr;
	}
}
