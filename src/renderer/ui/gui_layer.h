#pragma once

#include "renderer/core/layer/layer.h"

#include <SDL3/SDL_events.h>

namespace Moxel
{
    class GuiLayer final : public Layer
    {
    public:
        GuiLayer() = default;
        ~GuiLayer() override = default;

        void attach() override;
        void detach() override;

        static void process_events(const SDL_Event& event);
        static void begin();
        static void end();
    };
}