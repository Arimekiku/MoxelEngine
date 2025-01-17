#pragma once

namespace Moxel
{
	class Layer
	{
	public:
		Layer() = default;
		virtual ~Layer() = default;

		virtual void attach() { }
		virtual void detach() { }
		virtual void on_every_update() { }
		virtual void on_gui_update() { }
	};
}