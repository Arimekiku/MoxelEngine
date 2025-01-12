#include "window.h"
#include "renderer/ui/gui_layer.h"
#include "renderer/core/input.h"
#include "renderer/vulkan/vulkan_renderer.h"

namespace Moxel
{
	GameWindow::GameWindow(const int width, const int height)
	{
		SDL_Init(SDL_INIT_VIDEO);

		constexpr SDL_WindowFlags window_flags = SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE;

		m_nativeWindow = SDL_CreateWindow("Vulkan Engine", width, height, window_flags);
		m_windowSize = VkExtent2D(width, height);

		if (m_nativeWindow == nullptr)
		{
			SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not create window: %s\n", SDL_GetError());
		}
	}

	GameWindow::~GameWindow()
	{
		SDL_DestroyWindow(m_nativeWindow);
		SDL_Quit();
	}

	void GameWindow::update(LayerStack layers)
	{
		while (m_isOpen)
		{
			SDL_Event event;
			while (SDL_PollEvent(&event))
			{
				switch (event.type)
				{
					case SDL_EVENT_QUIT: m_isOpen = false; break;
					case SDL_EVENT_KEY_DOWN:
					{
						if (event.key.key == SDLK_ESCAPE)
						{
							m_isOpen = false;
						}

						Input::Key::set_key_value(event.key.key, true);
						break;
					}
					case SDL_EVENT_KEY_UP: Input::Key::set_key_value(event.key.key, false); break;
					case SDL_EVENT_MOUSE_BUTTON_UP: break; // TODO: implement
					default: break;
				}

				GuiLayer::process_events(event);
			}

			VulkanRenderer::prepare_frame();

			for (const auto layer : layers)
			{
				layer->on_every_update();
			}

			GuiLayer::begin();

			for (const auto layer : layers)
			{
				layer->on_gui_update();
			}

			GuiLayer::end();

			VulkanRenderer::end_frame();

			Input::Key::copy_new_layout();
		}
	}

	void GameWindow::update_window_size()
	{
		SDL_GetWindowSizeInPixels(m_nativeWindow, reinterpret_cast<int*>(&m_windowSize.width), reinterpret_cast<int*>(&m_windowSize.height));
	}
}
