#include "application.h"

namespace SDLarria {
	Application::Application() {
		//TODO : initialize log
		
		m_Engine = VulkanEngine();

		constexpr auto initialSize = VkExtent2D(1600, 900);
		m_Window = new GameWindow(initialSize);

		m_Engine.Initialize(m_Window->GetNativeWindow(), initialSize);
	}

	Application::~Application() {
		m_Engine.Shutdown();

		delete m_Window;
	}

	void Application::Run() {
		m_Window->Update(m_Engine);
	}
}