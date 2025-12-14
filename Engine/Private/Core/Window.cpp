#include "Core/Window.h"

#include "backends/imgui_impl_sdl3.h"
#include "Core/Engine.h"
#include "Core/WindowEvents.h"
#include "Input/FSDLInputSystem.h"

namespace Turbo
{
	FWindow::FWindow() = default;
	FWindow::~FWindow() = default;

	void FWindow::InitBackend()
	{
		TURBO_LOG(LOG_WINDOW, Info, "Initializing SDL.");
		if (!SDL_Init(SDL_INIT_VIDEO))
		{
			LogError();
		}
	}

	void FWindow::StopBackend()
	{
		TURBO_LOG(LOG_WINDOW, Info, "Stopping SDL.");
		SDL_Quit();
	}

	void FWindow::Destroy()
	{
		TURBO_LOG(LOG_WINDOW, Info, "Destroying window.");
		SDL_DestroyWindow(mSDLWindow);
	}

	bool FWindow::Init()
	{
		TURBO_LOG(LOG_WINDOW, Info, "Initializing Window.");
		mSDLWindow = SDL_CreateWindow(WindowDefaultValues::kName.data(), WindowDefaultValues::kSizeX, WindowDefaultValues::kSizeY,
		                                     SDL_WINDOW_VULKAN | SDL_WINDOW_HIGH_PIXEL_DENSITY | SDL_WINDOW_HIDDEN);
		if (!mSDLWindow)
		{
			TURBO_LOG(LOG_WINDOW, Error, "SDL window creation error. See bellow logs for details");
			LogError();

			return false;
		}

		return true;
	}

	void FWindow::LogError()
	{
		TURBO_LOG(LOG_WINDOW, Error, "SDL_ERROR: {}", SDL_GetError());
	}

	void FWindow::PollWindowEventsAndErrors()
	{
		SDL_Event event;

		// Handle events
		while (SDL_PollEvent(&event))
		{
			// todo: Move to engine subsystem
			// Handle ImGui events
			// ImGui_ImplSDL3_ProcessEvent(&event);

			switch (event.type)
			{
			case SDL_EVENT_QUIT:
			case SDL_EVENT_TERMINATING:
				{
					FCloseWindowEvent newEvent = {};
					gEngine->PushEvent(newEvent);
					break;
				}
			case SDL_EVENT_WINDOW_RESIZED:
				{
					FResizeWindowEvent newEvent = {};
					newEvent.mNewWindowSize = GetFrameBufferSize();
					gEngine->PushEvent(newEvent);
					break;
				}
			case SDL_EVENT_KEY_DOWN:
			case SDL_EVENT_KEY_UP:
				{
					OnSDLKeyboardEvent.ExecuteIfBound(event.key);
					break;
				}
			case SDL_EVENT_MOUSE_BUTTON_DOWN:
			case SDL_EVENT_MOUSE_BUTTON_UP:
				{
					OnSDLMouseButtonEvent.ExecuteIfBound(event.button);
					break;
				}
			case SDL_EVENT_MOUSE_MOTION:
				{
					OnSDLMouseMotionEvent.ExecuteIfBound(event.motion);
				}
			case SDL_EVENT_MOUSE_WHEEL:
				{
					OnSDLMouseWheelEvent.ExecuteIfBound(event.wheel);
				}
			default:
				break;
			}

			OnSDLEvent.Broadcast(&event);
		}
	}

	void FWindow::ShowWindow(bool bVisible)
	{
		TURBO_LOG(LOG_WINDOW, Info, "Setting window visibility to {}", bVisible);

		if (bVisible)
		{
			SDL_ShowWindow(mSDLWindow);
		}
		else
		{
			SDL_HideWindow(mSDLWindow);
		}
	}

	void FWindow::ShowCursor(bool bVisible)
	{
		TURBO_LOG(LOG_WINDOW, Display, "Setting cursor visibility to {}", bVisible);

		bool bResult = true;

		if (bVisible)
		{
			bResult &= SDL_ShowCursor();
		}
		else
		{
			bResult &= SDL_HideCursor();
		}

		bResult &= SDL_SetWindowRelativeMouseMode(mSDLWindow, !bVisible);

		if (bResult == false)
		{
			LogError();
		}
	}

	glm::uint2 FWindow::GetFrameBufferSize() const
	{
		glm::ivec2 Result;
		if (!SDL_GetWindowSizeInPixels(mSDLWindow, &Result.x, &Result.y))
		{
			LogError();
			Result = glm::ivec2(WindowDefaultValues::kSizeX, WindowDefaultValues::kSizeY);
		}

		return glm::ivec2(Result);
	}

	float FWindow::GetDisplayScale() const
	{
		const float displayScale = SDL_GetWindowDisplayScale(mSDLWindow);
		return displayScale > TURBO_SMALL_NUMBER ? displayScale : 1.f;
	}

	bool FWindow::IsFullscreenEnabled() const
	{
		return mbFullscreenEnabled;
	}

	void FWindow::SetFullscreen(bool bFullscreen)
	{
		if (!SDL_SetWindowFullscreen(mSDLWindow, bFullscreen))
		{
			LogError();
			return;
		}

		 mbFullscreenEnabled = bFullscreen;
	}

	void FWindow::InitForVulkan()
	{
		if (!SDL_Vulkan_LoadLibrary(nullptr))
		{
			LogError();
		}
	}

	void FWindow::DeInitForVulkan()
	{
		SDL_Vulkan_UnloadLibrary();
	}

	std::vector<const char*> FWindow::GetVulkanRequiredExtensions()
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

	bool FWindow::CreateVulkanSurface(VkInstance vulkanInstance)
	{
		if (mVulkanSurface)
		{
			return true;
		}

		if (!SDL_Vulkan_CreateSurface(mSDLWindow, vulkanInstance, nullptr, &mVulkanSurface))
		{
			TURBO_LOG(LOG_WINDOW, Error, "Window Vulkan surface creation error. Check bellow logs:");
			LogError();

			return false;
		}

		return true;
	}

	VkSurfaceKHR FWindow::GetVulkanSurface()
	{
		TURBO_CHECK(mVulkanSurface);

		return mVulkanSurface;
	}

	bool FWindow::DestroyVulkanSurface(VkInstance vulkanInstance)
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
