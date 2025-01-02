#include "engine/ui/gui_layer.h"
#include "editor/scene_layer.h"

#define VMA_IMPLEMENTATION
#include "engine/application.h"

int main(int argc, char** argv) 
{
    auto application = SDLarria::Application();

    application.AddLayer(new SDLarria::GuiLayer);
	application.AddLayer(new SDLarria::SceneLayer);

    application.Run();
}