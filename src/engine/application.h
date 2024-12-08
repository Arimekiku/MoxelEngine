#pragma once

#include "core/core.h"

namespace SDLarria 
{
	class Application 
	{
	public:
		Application();
		~Application();

		static Application& Get() { return *s_Instance; }

		void AddLayer(Layer* layer);

		void Run();

		GameWindow& GetWindow() { return *m_Window; }

	private:
		static Application* s_Instance;

		GameWindow* m_Window;
		VulkanEngine m_Engine;
		LayerStack m_LayerStack;
	};
}