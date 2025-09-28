#include "Services/ImGUIService.h"

#include "imgui.h"
#include "backends/imgui_impl_sdl3.h"
#include "backends/imgui_impl_vulkan.h"
#include "Core/Engine.h"
#include "Graphics/GPUDevice.h"

namespace Turbo
{
	void FImGuiLayer::OnSDLEvent(SDL_Event* sdlEvent)
	{
		if (sdlEvent->type == SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED)
		{
			const FWindow* window = gEngine->GetWindow();
			ImGui::GetStyle().ScaleAllSizes(window->GetDisplayScale());
		}

		ImGui_ImplSDL3_ProcessEvent(sdlEvent);
	}

	FName FImGuiLayer::GetName()
	{
		return mClassName;
	}

	void FImGuiLayer::Start()
	{
		FGPUDevice* gpu = gEngine->GetGpu();
		FWindow* window = gEngine->GetWindow();

		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

		ImGui_ImplSDL3_InitForVulkan(window->GetWindow());

		ImGui_ImplVulkan_InitInfo initInfo = {};
		initInfo.Instance = gpu->GetVkInstance();
		initInfo.PhysicalDevice = gpu->GetVkPhysicalDevice();
		initInfo.Device = gpu->GetVkDevice();
		initInfo.Queue = gpu->GetVkQueue();
		initInfo.DescriptorPoolSize = 128;
		initInfo.MinImageCount = 2;
		initInfo.ImageCount = 2;
		initInfo.UseDynamicRendering = true;

		initInfo.CheckVkResultFn = [](VkResult result) {CHECK_VULKAN_MSG(result, "ImGUI RHI Error.")};
		//dynamic rendering parameters for imgui to use
		initInfo.PipelineRenderingCreateInfo = {.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO};
		initInfo.PipelineRenderingCreateInfo.colorAttachmentCount = 1;

		const FTexture* presentTexture = gpu->AccessTexture(gpu->GetPresentImage());
		VkFormat presentTextureFormat = static_cast<VkFormat>(presentTexture->GetFormat());
		initInfo.PipelineRenderingCreateInfo.pColorAttachmentFormats = &presentTextureFormat;

		initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

		auto LoaderFunction = [](const char* functionName, void* userData)
		{
			const FGPUDevice* gpuDevice = static_cast<FGPUDevice*>(userData);
			return gpuDevice->GetVkInstance().getProcAddr(functionName);
		};

		ImGui_ImplVulkan_LoadFunctions(kVulkanVersion, LoaderFunction, gpu);
		ImGui_ImplVulkan_Init(&initInfo);

		gEngine->GetWindow()->OnSDLEvent.AddRaw(this, &FImGuiLayer::OnSDLEvent);

		ImFont* firaCodeImFont = io.Fonts->AddFontFromFileTTF("Content/Fonts/FiraCode/FiraCode-Regular.ttf", 13);
		ImGui::PushFont(firaCodeImFont, 13);
		ImGui::GetStyle().ScaleAllSizes(window->GetDisplayScale());
	}

	void FImGuiLayer::Shutdown()
	{
		gEngine->GetWindow()->OnSDLEvent.RemoveObject(this);

		FGPUDevice* gpu = gEngine->GetGpu();
		gpu->WaitIdle();

		ImGui_ImplSDL3_Shutdown();
		ImGui_ImplVulkan_Shutdown();
	}

	void FImGuiLayer::BeginTick_GameThread(float deltaTime)
	{
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplSDL3_NewFrame();
		ImGui::NewFrame();
	}

	void FImGuiLayer::EndTick_GameThread(float deltaTime)
	{
		ImGui::Render();
	}

	void FImGuiLayer::BeginPresentingFrame_RenderThread(FGPUDevice* gpu, FCommandBuffer* cmd, FTextureHandle PresentImage)
	{
		cmd->BeginRendering(PresentImage);
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd->GetVkCommandBuffer());
		cmd->EndRendering();
	}
} // Turbo