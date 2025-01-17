#pragma once

#include <functional>
#include <cstdint>

namespace Moxel
{
	class UUID
	{
	public:
		UUID();

		bool operator==(const UUID& check) const { return m_uniqueId == check.m_uniqueId; }
		operator uint32_t() const { return m_uniqueId; }
	private:
		uint32_t m_uniqueId = 0;
	};
}

template<>
struct std::hash<Moxel::UUID>
{
	std::size_t operator()(const Moxel::UUID& uuid) const noexcept
	{
		return uuid;
	}
};