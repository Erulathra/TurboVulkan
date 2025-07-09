#include "Core/Window.h"

#include "backends/imgui_impl_sdl3.h"

namespace Turbo
{
	FSDLWindow::FSDLWindow() = default;
	FSDLWindow::~FSDLWindow() = default;

	void FSDLWindow::InitBackend()
	{
		TURBO_LOG(LOG_WINDOW, LOG_INFO, "Initializing SDL.");
		if (!SDL_Init(SDL_INIT_VIDEO))
		{
			LogError();
		}
	}

	void FSDLWindow::StopBackend()
	{
		TURBO_LOG(LOG_WINDOW, LOG_INFO, "Stopping SDL.");
		SDL_Quit();
	}

	void FSDLWindow::Destroy()
	{
		TURBO_LOG(LOG_WINDOW, LOG_INFO, "Destroying window.");
		SDL_DestroyWindow(mSDLWindow);
	}

	bool FSDLWindow::Init()
	{
		TURBO_LOG(LOG_WINDOW, LOG_INFO, "Initializing Window.");
		mSDLWindow = SDL_CreateWindow(WindowDefaultValues::kName.c_str(), WindowDefaultValues::kSizeX, WindowDefaultValues::kSizeY,
		                                     SDL_WINDOW_VULKAN | SDL_WINDOW_HIGH_PIXEL_DENSITY | SDL_WINDOW_HIDDEN);
		if (!mSDLWindow)
		{
			TURBO_LOG(LOG_WINDOW, LOG_ERROR, "SDL window creation error. See bellow logs for details");
			LogError();

			return false;
		}

		return true;
	}

	void FSDLWindow::LogError()
	{
		TURBO_LOG(LOG_WINDOW, LOG_ERROR, "SDL_ERROR: {}", SDL_GetError());
	}

	void FSDLWindow::PollWindowEventsAndErrors()
	{
		SDL_Event event;

		// Handle events
		while (SDL_PollEvent(&event))
		{
			// Handle ImGui events
			ImGui_ImplSDL3_ProcessEvent(&event);

			if (event.type == SDL_EVENT_QUIT || event.type == SDL_EVENT_TERMINATING)
				OnWindowEvent.Broadcast(EWindowEvent::WindowCloseRequest);
			else if (event.type == SDL_EVENT_WINDOW_FOCUS_LOST)
				OnWindowEvent.Broadcast(EWindowEvent::FocusLost);
			else if (event.type == SDL_EVENT_WINDOW_FOCUS_GAINED)
				OnWindowEvent.Broadcast(EWindowEvent::FocusGained);
		}
	}

	void FSDLWindow::ShowWindow(bool bVisible)
	{
		TURBO_LOG(LOG_WINDOW, LOG_INFO, "Setting window visibility to {}", bVisible);

		if (bVisible)
		{
			SDL_ShowWindow(mSDLWindow);
		}
		else
		{
			SDL_HideWindow(mSDLWindow);
		}
	}

	glm::ivec2 FSDLWindow::GetFrameBufferSize() const
	{
		glm::ivec2 Result;
		if (!SDL_GetWindowSizeInPixels(mSDLWindow, &Result.x, &Result.y))
		{
			LogError();
			Result = glm::ivec2(WindowDefaultValues::kSizeX, WindowDefaultValues::kSizeY);
		}

		return glm::ivec2(Result);
	}

	void FSDLWindow::InitForVulkan()
	{
		if (!SDL_Vulkan_LoadLibrary(nullptr))
		{
			LogError();
		}
	}

	void FSDLWindow::DeInitForVulkan()
	{
		SDL_Vulkan_UnloadLibrary();
	}

	std::vector<const char*> FSDLWindow::GetVulkanRequiredExtensions()
	{
		std::vector<const char*> Result;

		uint32 ExtensionsCount;
		char const* const* ExtensionNames = SDL_Vulkan_GetInstanceExtensions(&ExtensionsCount);
		if (ExtensionNames == nullptr)
		{
			LogError();
			return Result;
		}

		for (int ExtensionId = 0; ExtensionId < ExtensionsCount; ++ExtensionId)
		{
			Result.push_back(ExtensionNames[ExtensionId]);
		}

		return Result;
	}

	bool FSDLWindow::CreateVulkanSurface(VkInstance vulkanInstance)
	{
		if (mVulkanSurface)
		{
			return true;
		}

		if (!SDL_Vulkan_CreateSurface(mSDLWindow, vulkanInstance, nullptr, &mVulkanSurface))
		{
			TURBO_LOG(LOG_WINDOW, LOG_ERROR, "Window Vulkan surface creation error. Check bellow logs:");
			LogError();

			return false;
		}

		return true;
	}

	VkSurfaceKHR FSDLWindow::GetVulkanSurface()
	{
		TURBO_CHECK(mVulkanSurface);

		return mVulkanSurface;
	}

	bool FSDLWindow::DestroyVulkanSurface(VkInstance vulkanInstance)
	{
		if (vulkanInstance)
		{
			SDL_Vulkan_DestroySurface(vulkanInstance, mVulkanSurface, nullptr);
			mVulkanSurface = nullptr;

			return true;
		}

		return false;
	}
} // Turbo
