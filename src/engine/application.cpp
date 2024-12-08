#include "application.h"
#include "engine/ui/gui_layer.h"

namespace SDLarria 
{
	Application* Application::s_Instance;

	Application::Application() 
	{
		Log::Initialize();
		
		m_Engine = VulkanEngine();

		constexpr auto initialSize = VkExtent2D(1600, 900);
		m_Window = new GameWindow(initialSize.width, initialSize.height);

		m_Engine.Initialize(m_Window->GetNativeWindow(), initialSize);

		s_Instance = this;

		m_LayerStack = LayerStack();
		
	}

	Application::~Application()
	{
		m_LayerStack.Clear();
		m_Engine.Shutdown();
	}

	void Application::Run()
	{
		m_Window->Update(m_Engine, m_LayerStack);
	}

	void Application::AddLayer(Layer* layer)
	{
		m_LayerStack.Push(layer);
		layer->Attach();
	}
}