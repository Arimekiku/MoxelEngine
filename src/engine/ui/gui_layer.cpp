#include "gui_layer.h"
#include "engine/vulkan/vulkan.h"
#include "engine/application.h"

#include <backends/imgui_impl_vulkan.cpp>
#include <backends/imgui_impl_sdl3.cpp> 

namespace SDLarria
{
    static VkDescriptorPool s_Test;

    void GuiLayer::Attach()
    {
		const auto& vulkan = Application::Get().GetContext();
        const VkDescriptorPoolSize pool_sizes[] =
        { 
            { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
            { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
            { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 } 
        };

        auto pool_info = VkDescriptorPoolCreateInfo();
        pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        pool_info.maxSets = 1000;
        pool_info.poolSizeCount = static_cast<uint32_t>(std::size(pool_sizes));
        pool_info.pPoolSizes = pool_sizes;

        const auto result = vkCreateDescriptorPool(vulkan.GetLogicalDevice(), &pool_info, nullptr, &s_Test);
        VULKAN_CHECK(result);

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

        const auto window = Application::Get().GetWindow().GetNativeWindow();
        ImGui_ImplSDL3_InitForVulkan(window);

        auto initInfo = ImGui_ImplVulkan_InitInfo();
        initInfo.Instance = vulkan.GetInstance();
        initInfo.PhysicalDevice = vulkan.GetPhysicalDevice();
        initInfo.Device = vulkan.GetLogicalDevice();
        initInfo.Queue = vulkan.GetRenderQueue();
        initInfo.DescriptorPool = s_Test;
        initInfo.MinImageCount = 3;
        initInfo.ImageCount = 3;
        initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
        initInfo.CheckVkResultFn = VulkanUtils::VulkanCheck;

        auto& swapchain = VulkanRenderer::Get().GetSwapchain();
        auto createInfo = VkPipelineRenderingCreateInfo();
        createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
        createInfo.colorAttachmentCount = 1;
        createInfo.pColorAttachmentFormats = &swapchain.GetImageFormat();

        initInfo.UseDynamicRendering = true;
        initInfo.PipelineRenderingCreateInfo = createInfo;

        ImGui_ImplVulkan_Init(&initInfo);
        ImGui_ImplVulkan_CreateFontsTexture();

        ImGui::StyleColorsDark();
    }

    void GuiLayer::Detach()
    {
        const auto device = Application::Get().GetContext().GetLogicalDevice();

        vkDeviceWaitIdle(device);

        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplSDL3_Shutdown();

        vkDestroyDescriptorPool(device, s_Test, nullptr);
        ImGui::DestroyContext();
    }

    void GuiLayer::ProcessEvents(const SDL_Event& event)
    {
        ImGui_ImplSDL3_ProcessEvent(&event);
    }

    void GuiLayer::Begin()
    {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();
    }

    void GuiLayer::End()
    {
        ImGui::Render();
    }
}