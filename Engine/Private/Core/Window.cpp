#include "Core/Window.h"

#include "Core/Engine.h"

#define DEFAULT_WINDOW_SIZE_X 640
#define DEFAULT_WINDOW_SIZE_Y 480
#define DEFAULT_WINDOW_NAME "Turbo Vulkan 🗻"

namespace Turbo {
	std::unique_ptr<Window> Window::MainWindow = nullptr;

	Window::Window()
	{
	}

	Window::~Window()
	{
		TURBO_LOG(LOG_WINDOW, LOG_INFO, "Destroing window");

		SDL_DestroyWindow(SDLContext.Window);

		if (this == MainWindow.get())
		{
			SDL_Quit();
		}

		LogErrors();
	}

	void Window::CreateMainWindow()
	{
		TURBO_LOG(LOG_WINDOW, LOG_INFO, "Initializing SDL");
		SDL_Init(SDL_INIT_VIDEO);
		LogErrors();

		MainWindow = std::unique_ptr<Window>(new Window());
		MainWindow->InitWindow();
	}

	void Window::DestroyMainWindow()
	{
		MainWindow.release();
	}

	void Window::InitWindow()
	{
		TURBO_LOG(LOG_WINDOW, LOG_INFO, "Initializing Window");
		SDLContext.Window = SDL_CreateWindow(DEFAULT_WINDOW_NAME, DEFAULT_WINDOW_SIZE_X, DEFAULT_WINDOW_SIZE_Y, SDL_WINDOW_VULKAN);

		LogErrors();
	}

	void Window::LogErrors()
	{
		const char* Error = SDL_GetError();
		while (strlen(Error) > 0)
		{
			TURBO_LOG(LOG_WINDOW, LOG_ERROR, "SDL_ERROR: {}", Error);
			Error = SDL_GetError();
		}
	}

	void Window::PollWindowEventsAndErrors()
	{
		SDL_Event windowEvent;
		while (SDL_PollEvent(&windowEvent))
		{
			if (windowEvent.type == SDL_EVENT_QUIT) OnWindowEvent(EWindowEvent::WindowCloseRequest);
			else if (windowEvent.type == SDL_EVENT_WINDOW_FOCUS_LOST) OnWindowEvent(EWindowEvent::FocusLost);
			else if (windowEvent.type == SDL_EVENT_WINDOW_FOCUS_GAINED) OnWindowEvent(EWindowEvent::FocusGained);
		}

		LogErrors();
	}
} // Turbo