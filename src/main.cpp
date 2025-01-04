#include "renderer/ui/gui_layer.h"
#include "editor/scene_layer.h"

#include "renderer/application.h"

int main(int argc, char** argv) 
{
	auto application = Moxel::Application();

	application.AddLayer(new Moxel::GuiLayer);
	application.AddLayer(new Moxel::SceneLayer);

	application.Run();
}