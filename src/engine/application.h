#pragma once

#include "core/core.h"

namespace SDLarria {
	class Application {
	public:
		Application();
		~Application();

		void Run();

	private:
		GameWindow* m_Window;
		VulkanEngine m_Engine;
	};
}