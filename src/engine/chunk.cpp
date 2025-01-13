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
	}

	void Chunk::generate_data(const ChunkPosition position)
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
	}

	ChunkMesh::~ChunkMesh()
	{
		destroy_mesh();
	}

	glm::mat4 ChunkMesh::get_trs_matrix(const ChunkPosition position)
	{
		const glm::mat4 translation = translate(glm::mat4(1.0f), glm::vec3(position.x, position.y, position.z) * 16.0f);
		const glm::mat4 rotation = toMat4(glm::quat(radians(glm::vec3(0))));
		const glm::mat4 scaling = scale(glm::mat4(1.0f), glm::vec3(1.0f));

		return translation * rotation * scaling;
	}

	void ChunkMesh::destroy_mesh()
	{
		if (m_chunkMesh == nullptr)
		{
			return;
		}

		m_chunkMesh = nullptr;
	}
}
