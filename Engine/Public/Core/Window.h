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
		inline static Window* GetMain() { return MainWindow.get(); }

		static void InitBackend();

		static bool CreateMainWindow();
		static void DestroyMainWindow();


		/** Events */
	public:
		WindowEventDelegate OnWindowEvent;

		/** Basic Interface */
	public:
		void PollWindowEventsAndErrors();

		void ShowWindow(bool bVisible);

		/** Vulkan Interface */
	public:
		static void InitForVulkan();

		void DeInitForVulkan();
		std::vector<const char*> GetVulkanExtensions();

		bool CreateVulkanSurface(VkInstance VulkanInstance, VkSurfaceKHR& OutVulkanSurface) const;
		bool DestroyVulkanSurface(VkInstance VulkanInstance, VkSurfaceKHR Surface);

		/** Internal methods */
	private:
		bool InitWindow();

		static void LogError();

		/** Static properties */
	private:
		static std::unique_ptr<Window> MainWindow;

		/** properties */
	private:
		struct
		{
			SDL_Window* Window = nullptr;
		} SDLContext;
	};
} // Turbo
