#include "window.h"
#include "engine/ui/gui_layer.h"

#include <iostream>
#include <backends/imgui_impl_sdl3.h>

namespace SDLarria 
{
	GameWindow::GameWindow(const int width, const int height)
    {
        SDL_Init(SDL_INIT_VIDEO);

        constexpr SDL_WindowFlags window_flags = SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE;
        m_NativeWindow = SDL_CreateWindow("Vulkan Engine", width, height, window_flags);
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

    void GameWindow::Update(VulkanRenderer& renderer, LayerStack layers)
    {
        bool is_open = true;

        while (is_open) 
        {
            SDL_Event event;
            while (SDL_PollEvent(&event)) 
            {
                if (event.type == SDL_EVENT_QUIT)
                {
                    is_open = false;
                    break;
                }

                if (event.type == SDL_EVENT_KEY_DOWN) 
                {
                    if (event.key.key == SDLK_ESCAPE) 
                    {
                        is_open = false;
                        break;
                    }
                }

                ImGui_ImplSDL3_ProcessEvent(&event);
            }

            for (const auto layer : layers)
            {
                layer->OnEveryUpdate();
            }

            GuiLayer::Begin();

            for (const auto layer : layers)
            {
                layer->OnGuiUpdate();
            }

            ImGui::ShowDemoWindow();

            GuiLayer::End();

            renderer.Draw();
        }
    }
}