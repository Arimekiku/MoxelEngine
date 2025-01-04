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

		m_NativeWindow = SDL_CreateWindow("Vulkan Engine", width, height, window_flags);
		m_WindowSize = VkExtent2D(width, height);

		if (m_NativeWindow == nullptr)
		{
			SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not create window: %s\n", SDL_GetError());
		}
	}

	GameWindow::~GameWindow()
	{
		SDL_DestroyWindow(m_NativeWindow);
		SDL_Quit();
	}

	void GameWindow::Update(LayerStack layers)
	{
		while (m_IsOpen)
		{
			SDL_Event event;
			while (SDL_PollEvent(&event))
			{
				switch (event.type)
				{
					case SDL_EVENT_QUIT: m_IsOpen = false; break;
					case SDL_EVENT_KEY_DOWN:
					{
						if (event.key.key == SDLK_ESCAPE)
						{
							m_IsOpen = false;
						}

						Input::Key::SetKeyValue(event.key.key, true);
						break;
					}
					case SDL_EVENT_KEY_UP: Input::Key::SetKeyValue(event.key.key, false); break;
					case SDL_EVENT_MOUSE_BUTTON_UP: break; // TODO: implement
					default: break;
				}

				GuiLayer::ProcessEvents(event);
			}

			VulkanRenderer::PrepareFrame();

			for (const auto layer : layers)
			{
				layer->OnEveryUpdate();
			}

			GuiLayer::Begin();

			for (const auto layer : layers)
			{
				layer->OnGuiUpdate();
			}

			GuiLayer::End();

			VulkanRenderer::EndFrame();

			Input::Key::CopyNewLayout();
		}
	}

	void GameWindow::UpdateWindowSize()
	{
		SDL_GetWindowSizeInPixels(m_NativeWindow, reinterpret_cast<int*>(&m_WindowSize.width), reinterpret_cast<int*>(&m_WindowSize.height));
	}
}
