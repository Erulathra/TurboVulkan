#include "Core/Window.h"

namespace Turbo
{
	std::unique_ptr<Window> Window::MainWindow = nullptr;

	Window::Window()
	{
	}

	Window::~Window()
	{
		TURBO_LOG(LOG_WINDOW, LOG_INFO, "Destroying window.");

		SDL_DestroyWindow(SDLWindow);
	}

	void Window::InitBackend()
	{
		TURBO_LOG(LOG_WINDOW, LOG_INFO, "Initializing SDL.");
		if (!SDL_Init(SDL_INIT_VIDEO))
		{
			LogError();
		}
	}

	bool Window::CreateMainWindow()
	{
		MainWindow = std::unique_ptr<Window>(new Window());
		return MainWindow->InitWindow();
	}

	void Window::DestroyMainWindow()
	{
		MainWindow.reset();

		TURBO_LOG(LOG_WINDOW, LOG_INFO, "Stopping SDL.");
		SDL_Quit();
	}

	bool Window::InitWindow()
	{
		TURBO_LOG(LOG_WINDOW, LOG_INFO, "Initializing Window.");
		SDLWindow = SDL_CreateWindow(WindowDefaultValues::Name.c_str(), WindowDefaultValues::SizeX, WindowDefaultValues::SizeY,
		                                     SDL_WINDOW_VULKAN | SDL_WINDOW_HIGH_PIXEL_DENSITY | SDL_WINDOW_HIDDEN);
		if (!SDLWindow)
		{
			TURBO_LOG(LOG_WINDOW, LOG_ERROR, "SDL window creation error. See bellow logs for details");
			LogError();

			return false;
		}

		return true;
	}

	void Window::LogError()
	{
		TURBO_LOG(LOG_WINDOW, LOG_ERROR, "SDL_ERROR: {}", SDL_GetError());
	}

	void Window::PollWindowEventsAndErrors()
	{
		SDL_Event WindowEvent;
		while (SDL_PollEvent(&WindowEvent))
		{
			if (WindowEvent.type == SDL_EVENT_QUIT || WindowEvent.type == SDL_EVENT_TERMINATING)
				OnWindowEvent(EWindowEvent::WindowCloseRequest);
			else if (WindowEvent.type == SDL_EVENT_WINDOW_FOCUS_LOST)
				OnWindowEvent(EWindowEvent::FocusLost);
			else if (WindowEvent.type == SDL_EVENT_WINDOW_FOCUS_GAINED)
				OnWindowEvent(EWindowEvent::FocusGained);
		}
	}

	void Window::ShowWindow(bool bVisible)
	{
		TURBO_LOG(LOG_WINDOW, LOG_INFO, "Setting window visibility to {}", bVisible);

		if (bVisible)
		{
			SDL_ShowWindow(SDLWindow);
		}
		else
		{
			SDL_HideWindow(SDLWindow);
		}
	}

	glm::uvec2 Window::GetFrameBufferSize() const
	{
		glm::ivec2 Result;
		if (!SDL_GetWindowSizeInPixels(SDLWindow, &Result.x, &Result.y))
		{
			LogError();
			Result = glm::ivec2{WindowDefaultValues::SizeX, WindowDefaultValues::SizeY};
		}

		return Result;
	}

	void Window::InitForVulkan()
	{
		if (!SDL_Vulkan_LoadLibrary(nullptr))
		{
			LogError();
		}
	}

	void Window::DeInitForVulkan()
	{
		SDL_Vulkan_UnloadLibrary();
	}

	std::vector<const char*> Window::GetVulkanRequiredExtensions()
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

	bool Window::CreateVulkanSurface(VkInstance VulkanInstance)
	{
		if (VulkanSurface)
		{
			return true;
		}

		if (!SDL_Vulkan_CreateSurface(SDLWindow, VulkanInstance, nullptr, &VulkanSurface))
		{
			TURBO_LOG(LOG_WINDOW, LOG_ERROR, "Window Vulkan surface creation error. Check bellow logs:");
			LogError();

			return false;
		}

		return true;
	}

	VkSurfaceKHR Window::GetVulkanSurface()
	{
		TURBO_CHECK(VulkanSurface);

		return VulkanSurface;
	}

	bool Window::DestroyVulkanSurface(VkInstance VulkanInstance)
	{
		if (VulkanInstance)
		{
			SDL_Vulkan_DestroySurface(VulkanInstance, VulkanSurface, nullptr);
			VulkanSurface = nullptr;

			return true;
		}

		return false;
	}
} // Turbo
