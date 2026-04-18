#include "Layers/ImGUILayer.h"

#include "imgui.h"
#include "backends/imgui_impl_sdl3.h"
#include "backends/imgui_impl_vulkan.h"
#include "Core/Engine.h"
#include "Core/FileSystem.h"
#include "Debug/IConsoleManager.h"
#include "Graphics/GPUDevice.h"
#include "UserInterface/UserInterfaceHelpers.h"

namespace Turbo
{
	void FImGuiLayer::OnSDLEvent(SDL_Event* sdlEvent)
	{
		if (sdlEvent->type == SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED)
		{
			const FWindow& window = entt::locator<FWindow>::value();
			ImGui::GetStyle().ScaleAllSizes(window.GetDisplayScale());
		}

		ImGui_ImplSDL3_ProcessEvent(sdlEvent);
	}

	void FImGuiLayer::SetupTheme()
	{
		// Hazy Dark style by kaitabuchi314 from ImThemes
		ImGuiStyle& style = ImGui::GetStyle();

		style.Alpha = 1.0f;
		style.DisabledAlpha = 0.6f;
		style.WindowPadding = ImVec2(5.5f, 8.3f);
		style.WindowRounding = 4.5f;
		style.WindowBorderSize = 1.0f;
		style.WindowMinSize = ImVec2(32.0f, 32.0f);
		style.WindowTitleAlign = ImVec2(0.0f, 0.5f);
		style.WindowMenuButtonPosition = ImGuiDir_Left;
		style.ChildRounding = 3.2f;
		style.ChildBorderSize = 1.0f;
		style.PopupRounding = 2.7f;
		style.PopupBorderSize = 1.0f;
		style.FramePadding = ImVec2(4.0f, 3.0f);
		style.FrameRounding = 2.4f;
		style.FrameBorderSize = 0.0f;
		style.ItemSpacing = ImVec2(8.0f, 4.0f);
		style.ItemInnerSpacing = ImVec2(4.0f, 4.0f);
		style.CellPadding = ImVec2(4.0f, 2.0f);
		style.IndentSpacing = 21.0f;
		style.ColumnsMinSpacing = 6.0f;
		style.ScrollbarSize = 14.0f;
		style.ScrollbarRounding = 9.0f;
		style.GrabMinSize = 10.0f;
		style.GrabRounding = 3.2f;
		style.TabRounding = 3.5f;
		style.TabBorderSize = 1.0f;
		style.ColorButtonPosition = ImGuiDir_Right;
		style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
		style.SelectableTextAlign = ImVec2(0.0f, 0.0f);

		style.Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
		style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.49803922f, 0.49803922f, 0.49803922f, 1.0f);
		style.Colors[ImGuiCol_WindowBg] = ImVec4(0.05882353f, 0.05882353f, 0.05882353f, 0.94f);
		style.Colors[ImGuiCol_ChildBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
		style.Colors[ImGuiCol_PopupBg] = ImVec4(0.078431375f, 0.078431375f, 0.078431375f, 0.94f);
		style.Colors[ImGuiCol_Border] = ImVec4(0.42745098f, 0.42745098f, 0.49803922f, 0.5f);
		style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
		style.Colors[ImGuiCol_FrameBg] = ImVec4(0.13725491f, 0.17254902f, 0.22745098f, 0.54f);
		style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.21176471f, 0.25490198f, 0.3019608f, 0.4f);
		style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.043137256f, 0.047058824f, 0.047058824f, 0.67f);
		style.Colors[ImGuiCol_TitleBg] = ImVec4(0.039215688f, 0.039215688f, 0.039215688f, 1.0f);
		style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.078431375f, 0.08235294f, 0.09019608f, 1.0f);
		style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.0f, 0.0f, 0.0f, 0.51f);
		style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.13725491f, 0.13725491f, 0.13725491f, 1.0f);
		style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.019607844f, 0.019607844f, 0.019607844f, 0.53f);
		style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.30980393f, 0.30980393f, 0.30980393f, 1.0f);
		style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.40784314f, 0.40784314f, 0.40784314f, 1.0f);
		style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.50980395f, 0.50980395f, 0.50980395f, 1.0f);
		style.Colors[ImGuiCol_CheckMark] = ImVec4(0.7176471f, 0.78431374f, 0.84313726f, 1.0f);
		style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.47843137f, 0.5254902f, 0.57254905f, 1.0f);
		style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.2901961f, 0.31764707f, 0.3529412f, 1.0f);
		style.Colors[ImGuiCol_Button] = ImVec4(0.14901961f, 0.16078432f, 0.1764706f, 0.4f);
		style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.13725491f, 0.14509805f, 0.15686275f, 1.0f);
		style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.078431375f, 0.08627451f, 0.09019608f, 1.0f);
		style.Colors[ImGuiCol_Header] = ImVec4(0.19607843f, 0.21568628f, 0.23921569f, 0.31f);
		style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.16470589f, 0.1764706f, 0.19215687f, 0.8f);
		style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.07450981f, 0.08235294f, 0.09019608f, 1.0f);
		style.Colors[ImGuiCol_Separator] = ImVec4(0.42745098f, 0.42745098f, 0.49803922f, 0.5f);
		style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.23921569f, 0.3254902f, 0.42352942f, 0.78f);
		style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.27450982f, 0.38039216f, 0.49803922f, 1.0f);
		style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.2901961f, 0.32941177f, 0.3764706f, 0.2f);
		style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.23921569f, 0.29803923f, 0.36862746f, 0.67f);
		style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.16470589f, 0.1764706f, 0.1882353f, 0.95f);
		style.Colors[ImGuiCol_Tab] = ImVec4(0.11764706f, 0.1254902f, 0.13333334f, 0.862f);
		style.Colors[ImGuiCol_TabHovered] = ImVec4(0.32941177f, 0.40784314f, 0.5019608f, 0.8f);
		style.Colors[ImGuiCol_TabActive] = ImVec4(0.24313726f, 0.24705882f, 0.25490198f, 1.0f);
		style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.06666667f, 0.101960786f, 0.14509805f, 0.9724f);
		style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.13333334f, 0.25882354f, 0.42352942f, 1.0f);
		style.Colors[ImGuiCol_PlotLines] = ImVec4(0.60784316f, 0.60784316f, 0.60784316f, 1.0f);
		style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.0f, 0.42745098f, 0.34901962f, 1.0f);
		style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.8980392f, 0.69803923f, 0.0f, 1.0f);
		style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.0f, 0.6f, 0.0f, 1.0f);
		style.Colors[ImGuiCol_TableHeaderBg] = ImVec4(0.1882353f, 0.1882353f, 0.2f, 1.0f);
		style.Colors[ImGuiCol_TableBorderStrong] = ImVec4(0.30980393f, 0.30980393f, 0.34901962f, 1.0f);
		style.Colors[ImGuiCol_TableBorderLight] = ImVec4(0.22745098f, 0.22745098f, 0.24705882f, 1.0f);
		style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
		style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.0f, 1.0f, 1.0f, 0.06f);
		style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.25882354f, 0.5882353f, 0.9764706f, 0.35f);
		style.Colors[ImGuiCol_DragDropTarget] = ImVec4(1.0f, 1.0f, 0.0f, 0.9f);
		style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.25882354f, 0.5882353f, 0.9764706f, 1.0f);
		style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.0f, 1.0f, 1.0f, 0.7f);
		style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.8f, 0.8f, 0.8f, 0.2f);
		style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.8f, 0.8f, 0.8f, 0.35f);
	}

	FName FImGuiLayer::GetName()
	{
		static FName Name("ImGUILayer");
		return Name;
	}

	void FImGuiLayer::Start()
	{
		FGPUDevice& gpu = entt::locator<FGPUDevice>::value();
		FWindow& window = entt::locator<FWindow>::value();

		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;

		const static std::string kConfigPath = FileSystem::PathCombine(FileSystem::kConfigPath, "ImGui.ini");
		const static std::string kLogPath = FileSystem::PathCombine(FileSystem::kConfigPath, "ImGui.txt");

		io.IniFilename = kConfigPath.c_str();
		io.LogFilename = kLogPath.c_str();

		ImGui_ImplSDL3_InitForVulkan(window.GetWindow());

		ImGui_ImplVulkan_InitInfo initInfo = {};
		initInfo.Instance = gpu.GetVkInstance();
		initInfo.PhysicalDevice = gpu.GetVkPhysicalDevice();
		initInfo.Device = gpu.GetVkDevice();
		initInfo.Queue = gpu.GetVkQueue();
		initInfo.DescriptorPoolSize = 128;
		initInfo.MinImageCount = 2;
		initInfo.ImageCount = 2;
		initInfo.UseDynamicRendering = true;

		initInfo.CheckVkResultFn = [](VkResult result) { CHECK_VULKAN_MSG(result, "ImGUI RHI Error.") };
		//dynamic rendering parameters for imgui to use
		initInfo.PipelineRenderingCreateInfo = {.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO};
		initInfo.PipelineRenderingCreateInfo.colorAttachmentCount = 1;

		const FTextureCold* presentTextureCold = gpu.AccessTextureCold(gpu.GetPresentImage());
		VkFormat presentTextureFormat = static_cast<VkFormat>(presentTextureCold->GetFormat());
		initInfo.PipelineRenderingCreateInfo.pColorAttachmentFormats = &presentTextureFormat;

		initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

		auto LoaderFunction = [](const char* functionName, void* userData)
		{
			const FGPUDevice* gpuDevice = static_cast<FGPUDevice*>(userData);
			return gpuDevice->GetVkInstance().getProcAddr(functionName);
		};

		ImGui_ImplVulkan_LoadFunctions(kVulkanVersion, LoaderFunction, &gpu);
		ImGui_ImplVulkan_Init(&initInfo);

		window.OnSDLEvent.AddRaw(this, &FImGuiLayer::OnSDLEvent);

		ImFont* firaCodeImFont = io.Fonts->AddFontFromFileTTF("Content/Fonts/FiraCode/FiraCode-Regular.ttf", 13);
		ImGui::PushFont(firaCodeImFont, 13);

		SetupTheme();

		ImGui::GetStyle().ScaleAllSizes(window.GetDisplayScale());
	}

	void FImGuiLayer::Shutdown()
	{
		FWindow& window = entt::locator<FWindow>::value();
		window.OnSDLEvent.RemoveObject(this);

		FGPUDevice& gpu = entt::locator<FGPUDevice>::value();
		gpu.WaitIdle();

		ImGui_ImplSDL3_Shutdown();
		ImGui_ImplVulkan_Shutdown();
	}

	void FImGuiLayer::BeginTick(double deltaTime)
	{
		TRACE_ZONE_SCOPED()

		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplSDL3_NewFrame();
		ImGui::NewFrame();

		ImGui::DockSpaceOverViewport(ImGui::GetID(kViewportDockspaceName), ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);
	}

	void FImGuiLayer::EndTick(double deltaTime)
	{
		TRACE_ZONE_SCOPED()

		ImGui::Render();
	}

	void FImGuiLayer::BeginPresentingFrame(FRenderGraphBuilder& graphBuilder, FRGResourceHandle presentImage)
	{
		ILayer::BeginPresentingFrame(graphBuilder, presentImage);

		static FName ImGUIPassName("RenderImGUI");
		graphBuilder.AddPass(
			ImGUIPassName,
			FRGSetupPassDelegate::CreateLambda(
				[&](FRGPassInfo& passInfo)
				{
					passInfo.mPassType = EPassType::Graphics;
					passInfo.AddAttachment(presentImage, 0);
					passInfo.ReadTexture(presentImage);
				}),
			FRGExecutePassDelegate::CreateLambda(
				[](FGPUDevice& gpu, FCommandBuffer& cmd, FRenderResources& resources)
				{
					TRACE_ZONE_SCOPED_N("Rendering ImGui")
					TRACE_GPU_SCOPED(gpu, cmd, "Rendering ImGUI")
					ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd.GetVkCommandBuffer());
				})
		);
	}
} // Turbo
