#pragma once

#include "layer/layer_stack.h"

#include <SDL3/SDL.h>
#include <vulkan/vulkan_core.h>

namespace SDLarria 
{
	class GameWindow 
	{
	public:
		GameWindow(int width, int height);
		~GameWindow();

		VkExtent2D GetWindowSize() const { return m_WindowSize; }
		const SDL_Surface* GetWindowSurface() const { return m_WindowSurface; }
		SDL_Window* GetNativeWindow() const { return m_NativeWindow; }

		void Update(LayerStack layers);
		void UpdateWindowSize();

	private:
		VkExtent2D m_WindowSize = VkExtent2D(0, 0);

		SDL_Window* m_NativeWindow = nullptr;
		SDL_Surface* m_WindowSurface = nullptr;

		bool m_IsOpen = true;
	};
}
