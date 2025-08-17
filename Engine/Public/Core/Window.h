#pragma once

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include "Core/Delegate.h"

namespace Turbo
{
	enum class EWindowEvent : uint32
	{
		None = 0,

		WindowCloseRequest,
		FocusLost,
		FocusGained,

		WindowResized,

		Num
	};

	// TODO: Replace that with config
	namespace WindowDefaultValues
	{
		static constexpr int32 kSizeX = 1280;
		static constexpr int32 kSizeY = 720;
		static constexpr std::string kName = "Turbo Vulkan";
	}

	DECLARE_MULTICAST_DELEGATE(FWindowEventDelegate, EWindowEvent);
	DECLARE_DELEGATE(FOnSDLKeyboardEvent, const SDL_KeyboardEvent&);

	class FWindow
	{
		GENERATED_BODY(FWindow)

		/** Constexpr */
	public:
		/** Statics */

		/** Constructors */
	private:
		explicit FWindow();

	public:
		~FWindow();

		/** Static Interface */
	public:
		void InitBackend();
		void StopBackend();

		bool Init();
		void Destroy();

		/** Events */
	public:
		FWindowEventDelegate OnWindowEvent;

	public:
		FWindowEventDelegate& GetEventTypeDelegate(EWindowEvent EventType) { return PerTypeWindowEvents[static_cast<uint32>(EventType)]; }

	private:
		std::vector<FWindowEventDelegate> PerTypeWindowEvents;


		/** Basic Interface */
	public:
		void PollWindowEventsAndErrors();

		void ShowWindow(bool bVisible);
		[[nodiscard]] glm::ivec2 GetFrameBufferSize() const;
		[[nodiscard]] SDL_Window* GetWindow() const { return mSDLWindow; }

		[[nodiscard]] bool IsFullscreenEnabled() const;
		void SetFullscreen(bool bFullscreen);

		/** SDL Interface **/
	public:
		void BindKeyboardEvent(const FOnSDLKeyboardEvent& NewDelegate) { OnSdlKeyboardEvent = NewDelegate; }
		void RemoveKeyboardEvent() { OnSdlKeyboardEvent = FOnSDLKeyboardEvent(); }

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

		FOnSDLKeyboardEvent OnSdlKeyboardEvent;

		bool mbFullscreenEnabled = false;

		// bool bFullscreen

	public:
		friend class FEngine;
	};
} // Turbo
