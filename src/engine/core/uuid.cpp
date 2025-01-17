#include "uuid.h"

#include <random>

namespace Moxel
{
	static std::random_device s_randomDevice;
	static auto s_machine = std::mt19937(s_randomDevice());
	static std::uniform_int_distribution<uint32_t> s_distribution;

	UUID::UUID()
	{
		m_uniqueId = s_distribution(s_machine);
	}
}
