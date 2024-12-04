#pragma once

#include "window.h"
#include "vulkan/vulkan_engine.h"

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