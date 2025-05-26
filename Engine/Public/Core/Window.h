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
		static constexpr int32 SizeX = 640;
		static constexpr int32 SizeY = 480;
		static constexpr std::string Name = "Turbo Vulkan";
	}

	using WindowEventDelegate = eventpp::CallbackList<void(EWindowEvent)>;

	class Window
	{
		/** Constexpr */
	public:
		/** Statics */

		/** Constructors */
	private:
		explicit Window();

	public:
		~Window();

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
		glm::uvec2 GetFrameBufferSize() const;

		/** Vulkan Interface */
	public:
		void InitForVulkan();

		void DeInitForVulkan();
		std::vector<const char*> GetVulkanRequiredExtensions();

		bool CreateVulkanSurface(VkInstance VulkanInstance);
		VkSurfaceKHR GetVulkanSurface();
		bool DestroyVulkanSurface(VkInstance VulkanInstance);

		/** Internal methods */
	private:

		static void LogError();

		/** properties */
	private:
		SDL_Window* SDLWindow = nullptr;
		VkSurfaceKHR VulkanSurface = nullptr;

	public:
		friend class Engine;
	};
} // Turbo
