#include "application.h"
#include "engine/ui/gui_layer.h"

namespace SDLarria 
{
	Application* Application::s_Instance;

	Application::Application()
	{
		s_Instance = this;

		Log::Initialize();
		
		m_Engine = VulkanRenderer();

		constexpr auto initialSize = VkExtent2D(1600, 900);
		m_Window = new GameWindow(initialSize.width, initialSize.height);
		m_Context.Initialize(m_Window->GetNativeWindow());

		VulkanRenderer::Initialize(initialSize);

		m_LayerStack = LayerStack();
	}

	Application::~Application()
	{
		vkDeviceWaitIdle(m_Context.GetLogicalDevice());

		m_LayerStack.Clear();

		VulkanRenderer::Shutdown();
		VulkanAllocator::Destroy();

		m_Context.Destroy();
	}

	void Application::Run()
	{
		m_Window->Update(m_LayerStack);
	}

	void Application::AddLayer(Layer* layer)
	{
		m_LayerStack.Push(layer);
		layer->Attach();
	}
}