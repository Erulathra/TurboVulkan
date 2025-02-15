#pragma once

#include <SDL3/SDL.h>
#include <eventpp/eventqueue.h>

namespace Turbo
{
	enum class EWindowEvent : uint32_t
	{
		WindowCloseRequest,
		FocusLost,
		FocusGained,
	};

	using WindowEventDelegate = eventpp::CallbackList<void(EWindowEvent)>;

	class Window
	{
		/** Constructors */
	private:
		explicit Window();

	public:
		~Window();

		/** Static Interface */
	public:
		inline static Window* GetMain() { return MainWindow.get(); }
		static void CreateMainWindow();
		static void DestroyMainWindow();

		/** Events */
	public:
		WindowEventDelegate OnWindowEvent;

		/** Basic Interface */
	public:
		void PollWindowEventsAndErrors();

		/** Internal methods */
	private:
		void InitWindow();

		static void LogErrors();

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
