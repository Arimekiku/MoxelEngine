#include "renderer/ui/gui_layer.h"
#include "editor/scene_layer.h"
#include "renderer/application.h"

int main(int argc, char** argv) 
{
	auto application = Moxel::Application();

	application.add_layer(new Moxel::GuiLayer);
	application.add_layer(new Moxel::SceneLayer);

	application.run();
}