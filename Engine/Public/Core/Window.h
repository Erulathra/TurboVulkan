#pragma once

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include "Core/Delegate.h"

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
		[[nodiscard]] glm::uint2 GetFrameBufferSize() const;
		[[nodiscard]] SDL_Window* GetWindow() const { return mSDLWindow; }

		float GetDisplayScale() const;

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
