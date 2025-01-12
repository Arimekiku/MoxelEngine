#pragma once

#include "uuid.h"

#include <memory>

namespace Moxel
{
	class Asset
	{
	public:
		Asset() = default;
		virtual ~Asset() = default;

		UUID get_uuid() const { return m_uuid; }

		template<class T>
		std::shared_ptr<T> as()	const { return std::dynamic_pointer_cast<T>(this); }
	private:
		UUID m_uuid;
	};
}