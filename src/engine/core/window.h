#pragma once

#include "layer/layer_stack.h"

#include <SDL3/SDL.h>
#include <vulkan/vulkan_core.h>

namespace Moxel
{
	class GameWindow 
	{
	public:
		GameWindow(int width, int height);
		~GameWindow();

		VkExtent2D get_window_size() const { return m_windowSize; }
		const SDL_Surface* get_window_surface() const { return m_windowSurface; }
		SDL_Window* get_native_window() const { return m_nativeWindow; }

		void update(LayerStack layers);
		void update_window_size();

	private:
		VkExtent2D m_windowSize = VkExtent2D(0, 0);

		SDL_Window* m_nativeWindow = nullptr;
		SDL_Surface* m_windowSurface = nullptr;

		bool m_isOpen = true;
	};
}
