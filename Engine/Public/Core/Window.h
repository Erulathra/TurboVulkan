#pragma once

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <eventpp/eventqueue.h>

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
		static constexpr int32 kSizeX = 640;
		static constexpr int32 kSizeY = 480;
		static constexpr std::string kName = "Turbo Vulkan";
	}

	using WindowEventDelegate = eventpp::CallbackList<void(EWindowEvent)>;

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
		WindowEventDelegate OnWindowEvent;

		/** Basic Interface */
	public:
		void PollWindowEventsAndErrors();

		void ShowWindow(bool bVisible);
		FUIntVector2 GetFrameBufferSize() const;

		/** Vulkan Interface */
	public:
		void InitForVulkan();

		void DeInitForVulkan();
		std::vector<const char*> GetVulkanRequiredExtensions();

		bool CreateVulkanSurface(VkInstance vulkanInstance);
		VkSurfaceKHR GetVulkanSurface();
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
