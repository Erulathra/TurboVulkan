#pragma once

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include "Core/Delegate.h"

namespace Turbo
{
	enum class EWindowEvent : uint32_t
	{
		WindowCloseRequest,
		FocusLost,
		FocusGained,
	};

	// TODO: Replace that with config
	namespace WindowDefaultValues
	{
		static constexpr int32 kSizeX = 1280;
		static constexpr int32 kSizeY = 720;
		static constexpr std::string kName = "Turbo Vulkan";
	}

	DECLARE_MULTICAST_DELEGATE(FWindowEventDelegate, EWindowEvent);

	class FSDLWindow
	{
		/** Constexpr */
	public:
		/** Statics */

		/** Constructors */
	private:
		explicit FSDLWindow();

	public:
		~FSDLWindow();

		/** Static Interface */
	public:
		void InitBackend();
		void StopBackend();

		bool Init();
		void Destroy();

		/** Events */
	public:
		FWindowEventDelegate OnWindowEvent;

		/** Basic Interface */
	public:
		void PollWindowEventsAndErrors();

		void ShowWindow(bool bVisible);
		[[nodiscard]] glm::ivec2 GetFrameBufferSize() const;
		[[nodiscard]] SDL_Window* GetWindow() const { return mSDLWindow; }

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

	public:
		friend class FEngine;
	};
} // Turbo
