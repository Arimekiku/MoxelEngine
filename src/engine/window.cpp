#include "window.h"
#include "image.h"
#include "vulkan/vulkan_engine.h"

#include <iostream>

namespace SDLarria {
	GameWindow::GameWindow(VkExtent2D size) {
        SDL_Init(SDL_INIT_VIDEO);

        constexpr SDL_WindowFlags window_flags = SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE;
        const int width = size.width;
        const int height = size.height;

        m_NativeWindow = SDL_CreateWindow("Vulkan Engine", width, height, window_flags);
        if (m_NativeWindow == nullptr) {
            SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not create window: %s\n", SDL_GetError());
            return;
        }
	}

    GameWindow::~GameWindow()
    {
        SDL_DestroyWindow(m_NativeWindow);
        SDL_Quit();
    }

    void GameWindow::Update(VulkanEngine& engine)
    {
        bool is_open = true;

        while (is_open) {
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_EVENT_QUIT) {
                    is_open = false;
                }

                if (event.type == SDL_EVENT_KEY_DOWN) {
                    if (event.key.key == SDLK_ESCAPE) {
                        is_open = false;
                    }
                }

                if (event.type == SDL_EVENT_WINDOW_RESIZED) {
                    //m_WindowSurface = SDL_GetWindowSurface(m_NativeWindow);
                }
            }

            engine.Draw();
        }
    }
}