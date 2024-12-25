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

		const SDL_Surface* GetWindowSurface() const { return m_WindowSurface; }
		SDL_Window* GetNativeWindow() const { return m_NativeWindow; }

		void Update(VulkanRenderer& renderer, LayerStack layers);

	private:
		SDL_Window* m_NativeWindow = nullptr;
		SDL_Surface* m_WindowSurface = nullptr;
	};
}