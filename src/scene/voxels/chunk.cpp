#include "chunk.h"

#include <PerlinNoise.hpp>

#define GLM_ENABLE_EXPERIMENTAL
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
		const auto perlin = siv::PerlinNoise(123456u);
		for (int z = 0; z < chunkSize; ++z)
		{
			for (int y = 0; y < chunkSize; ++y)
			{
				for (int x = 0; x < chunkSize; ++x)
				{
					const auto offset = glm::vec3(position.X * chunkSize + x, position.Y * chunkSize + y, position.Z * chunkSize + z);
					const double noise = perlin.octave3D_01(offset.x * 0.01f, offset.y * 0.01f, offset.z * 0.01f, 4);

					if (noise > 0.5f)
						set_block(z * chunkSize * chunkSize + y * chunkSize + x);
				}
			}
		}

		m_isProcessed = true;
	}

	ChunkMesh::~ChunkMesh()
	{
		clear_mesh();
	}

	void ChunkMesh::clear_mesh()
	{
		if (m_chunkMesh == nullptr)
			return;

		m_chunkMesh = nullptr;
	}
}
