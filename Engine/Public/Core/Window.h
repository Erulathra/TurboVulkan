#pragma once

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include "Core/Delegate.h"

DECLARE_LOG_CATEGORY(LogWindow, Info, Display)

namespace Turbo
{
	// TODO: Replace that with config
	namespace WindowDefaultValues
	{
		static constexpr int32 kSizeX = 1280;
		static constexpr int32 kSizeY = 720;
		static constexpr std::string_view kName = "Turbo Vulkan";
	}

	DECLARE_MULTICAST_DELEGATE(FSDLEventDelegate, SDL_Event*);
	DECLARE_DELEGATE(FOnSDLKeyboardEvent, const SDL_KeyboardEvent&);
	DECLARE_DELEGATE(FOnSDLMouseButtonEvent, const SDL_MouseButtonEvent&);
	DECLARE_DELEGATE(FOnSDLMouseMotionEvent, const SDL_MouseMotionEvent&);
	DECLARE_DELEGATE(FOnSDLMouseWheelEvent, const SDL_MouseWheelEvent&);

	class FWindow
	{
		/** Constexpr */
	public:
		/** Statics */

		/** Constructors */
	private:
		explicit FWindow();

	public:
		DELETE_COPY(FWindow);
		~FWindow();

		/** Static Interface */
	public:
		void InitBackend();
		void StopBackend();

		bool Init();
		void Destroy();

		/** Events */
	public:
		FSDLEventDelegate OnSDLEvent;

		/** Basic Interface */
	public:
		void PollWindowEventsAndErrors();

		void ShowWindow(bool bVisible);
		void ShowCursor(bool bVisible);

		[[nodiscard]] glm::uint2 GetFrameBufferSize() const;
		[[nodiscard]] SDL_Window* GetWindow() const { return mSDLWindow; }

		float GetDisplayScale() const;

		[[nodiscard]] bool IsFullscreenEnabled() const;
		void SetFullscreen(bool bFullscreen);

		/** SDL Interface **/
	public:
		void BindKeyboardEvent(const FOnSDLKeyboardEvent& NewDelegate) { OnSDLKeyboardEvent = NewDelegate; }
		void RemoveKeyboardEvent() { OnSDLKeyboardEvent = FOnSDLKeyboardEvent(); }

		void BindMouseButtonEvent(const FOnSDLMouseButtonEvent& NewDelegate) { OnSDLMouseButtonEvent = NewDelegate; }
		void RemoveMouseButtonEvent() { OnSDLMouseButtonEvent = FOnSDLMouseButtonEvent(); }

		void BindMouseMotionEvent(const FOnSDLMouseMotionEvent& NewDelegate) { OnSDLMouseMotionEvent = NewDelegate; }
		void RemoveMouseMotionEvent() { OnSDLMouseMotionEvent = FOnSDLMouseMotionEvent(); }

		void BindMouseWheelEvent(const FOnSDLMouseWheelEvent& NewDelegate) { OnSDLMouseWheelEvent = NewDelegate; }
		void RemoveMouseWheelEvent() { OnSDLMouseWheelEvent = FOnSDLMouseWheelEvent(); }

		/** Vulkan Interface */
	public:
		void InitForVulkan();

		void DeInitForVulkan();
		[[nodiscard]] std::vector<const char*> GetVulkanRequiredExtensions();

		bool CreateVulkanSurface(VkInstance vulkanInstance);
		[[nodiscard]] VkSurfaceKHR GetVulkanSurface();
		bool DestroyVulkanSurface(VkInstance vulkanInstance);

		/** Internal methods */
	private:
		static void LogError();

		/** properties */
	private:
		SDL_Window* mSDLWindow = nullptr;
		VkSurfaceKHR mVulkanSurface = nullptr;

		FOnSDLKeyboardEvent OnSDLKeyboardEvent;
		FOnSDLMouseButtonEvent OnSDLMouseButtonEvent;
		FOnSDLMouseMotionEvent OnSDLMouseMotionEvent;
		FOnSDLMouseWheelEvent OnSDLMouseWheelEvent;

		bool mbFullscreenEnabled = false;

		// bool bFullscreen

	public:
		friend class FEngine;
	};


} // Turbo
