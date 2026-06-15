#include "Core/Window.h"

#include "backends/imgui_impl_sdl3.h"
#include "Core/Engine.h"
#include "Core/FileSystem.h"
#include "Core/WindowEvents.h"
#include "Input/FSDLInputSystem.h"

#include "STB/stb_image.h"

namespace Turbo
{
	FWindow::FWindow() = default;
	FWindow::~FWindow() = default;

	void FWindow::InitBackend()
	{
		TURBO_LOG(LogWindow, Info, "Initializing SDL.");
		if (!SDL_Init(SDL_INIT_VIDEO))
		{
			LogError();
		}
	}

	void FWindow::StopBackend()
	{
		TURBO_LOG(LogWindow, Info, "Stopping SDL.");
		SDL_Quit();
	}

	void FWindow::Destroy()
	{
		TURBO_LOG(LogWindow, Info, "Destroying window.");
		SDL_DestroyWindow(mSDLWindow);
	}

	bool FWindow::Init()
	{
		TURBO_LOG(LogWindow, Info, "Initializing Window.");
		mSDLWindow = SDL_CreateWindow(WindowDefaultValues::kName.data(), WindowDefaultValues::kSizeX, WindowDefaultValues::kSizeY,
		                                     SDL_WINDOW_VULKAN | SDL_WINDOW_HIGH_PIXEL_DENSITY | SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE);
		if (!mSDLWindow)
		{
			TURBO_LOG(LogWindow, Error, "SDL window creation error. See bellow logs for details");
			LogError();

			return false;
		}

		return true;
	}

	void FWindow::LogError()
	{
		TURBO_LOG(LogWindow, Error, "SDL_ERROR: {}", SDL_GetError());
	}

	SDL_Surface* FWindow::LoadSurface(std::string_view path)
	{
		std::vector<byte> imgData;
		if (FileSystem::LoadData(path, imgData) == false)
		{
			return nullptr;
		}

		int32 sizeX, sizeY, numComponents;
#if 0
		void* pixels = stbi_load_from_memory(
			reinterpret_cast<stbi_uc*>(imgData.data()),
			imgData.size(),
			&sizeX,
			&sizeY,
			&numComponents,
			0
		);
#else
		void* pixels = stbi_load(
			path.data(),
			&sizeX,
			&sizeY,
			&numComponents,
			0
		);
#endif

		if (pixels == nullptr)
		{
			return nullptr;
		}

		SDL_Surface* result = SDL_CreateSurfaceFrom(
			sizeX,
			sizeY,
			numComponents == 3 ? SDL_PIXELFORMAT_RGB24 : SDL_PIXELFORMAT_RGBA32,
			pixels,
			sizeX * numComponents
		);

		stbi_image_free(pixels);

		if (result == nullptr)
		{
			return nullptr;
		}

		return result;
	}

	void FWindow::PollWindowEventsAndErrors()
	{
		SDL_Event event;

		// Handle events
		while (SDL_PollEvent(&event))
		{
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
					break;
				}
			case SDL_EVENT_MOUSE_WHEEL:
				{
					OnSDLMouseWheelEvent.ExecuteIfBound(event.wheel);
					break;
				}
			default:
				break;
			}

			OnSDLEvent.Broadcast(&event);
		}
	}

	void FWindow::ShowWindow(bool bVisible)
	{
		TURBO_LOG(LogWindow, Info, "Setting window visibility to {}", bVisible);

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
		TURBO_LOG(LogWindow, Display, "Setting cursor visibility to {}", bVisible);

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

	void FWindow::SetWindowIcon(std::string_view path)
	{
		TURBO_LOG(LogWindow, Info, "Setting window icon {}", path);

		if (mWindowIconSurface)
		{
			TURBO_LOG(LogWindow, Info, "Destroying old window icon.", path);
			SDL_DestroySurface(mWindowIconSurface);
			mWindowIconSurface = nullptr;
		}

		mWindowIconSurface = LoadSurface(path.data());

		SDL_SetWindowIcon(mSDLWindow, mWindowIconSurface);
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
			TURBO_LOG(LogWindow, Error, "Window Vulkan surface creation error. Check bellow logs:");
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
