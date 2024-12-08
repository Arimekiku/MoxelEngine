#pragma once

#include "engine/core/layer/layer.h"

namespace SDLarria
{
    class GuiLayer : public Layer
    {
    public:
        GuiLayer() = default;
        ~GuiLayer() override = default;

        void Attach() override;
        void Detach() override;

        static void Begin();
        static void End();
    };
}