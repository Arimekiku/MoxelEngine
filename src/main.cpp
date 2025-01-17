#include "engine/application.h"
#include "engine/ui/gui_layer.h"
#include "scene/scene_layer.h"

int main(int argc, char** argv) 
{
	auto application = Moxel::Application();

	application.add_layer(new Moxel::GuiLayer);
	application.add_layer(new Moxel::SceneLayer);

	application.run();
}