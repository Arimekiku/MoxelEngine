#pragma once

#include "engine/vulkan/vulkan_renderer.h"
#include "layer/layer_stack.h"

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
		void Resize();

	private:
		VkExtent2D m_WindowSize = VkExtent2D(0, 0);

		SDL_Window* m_NativeWindow = nullptr;
		SDL_Surface* m_WindowSurface = nullptr;
	};
}