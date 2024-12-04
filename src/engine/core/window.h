#pragma once

#include "engine/vulkan/vulkan_engine.h"

namespace SDLarria {
	class GameWindow {
	public:
		GameWindow(VkExtent2D size);
		~GameWindow();

		const SDL_Surface* GetWindowSurface() { return m_WindowSurface; }
		SDL_Window* GetNativeWindow() { return m_NativeWindow; }

		void Update(VulkanEngine& engine);

	private:
		SDL_Window* m_NativeWindow = nullptr;
		SDL_Surface* m_WindowSurface = nullptr;
	};
}