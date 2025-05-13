#include "Core/Engine.h"

#include "../../Public/Core/RHI/VulkanRHI.h"
#include "Core/Window.h"

namespace Turbo
{
	std::unique_ptr<Engine> Engine::Instance = nullptr;

	Engine::Engine()
		: bExitRequested(false)
	{
	}

	Engine::~Engine()
	{
		if (Window* MainWindow = Window::GetMain())
		{
			MainWindow->OnWindowEvent.remove(HandleMainWindowEventsHandle);
		}
	}

	void Engine::Init()
	{
		TURBO_LOG(LOG_ENGINE, LOG_INFO, "Creating engine instance.");
		Instance = std::unique_ptr<Engine>(new Engine());

		// TODO: Move thread configuration to separate class
		pthread_setname_np(pthread_self(), "GameThread");
	}

	Engine* Engine::Get()
	{
		return Instance.get();
	}

	int32_t Engine::Start(int32 argc, char* argv[])
	{
		// TODO: Move me elsewhere
		spdlog::set_level(spdlog::level::debug);

		Window::InitBackend();
		Window::InitForVulkan();

		if (!Window::CreateMainWindow())
		{
			return static_cast<int32_t>(EExitCode::WindowCreationError);
		}

		Window::GetMain()->OnWindowEvent.append(
			[this](EWindowEvent WindowEvent)
			{
				HandleMainWindowEvents(WindowEvent);
			});

		VulkanRHI::Init();

		Window::GetMain()->ShowWindow(true);

		GameThreadLoop();

		return static_cast<int32_t>(ExitCode);
	}

	void Engine::GameThreadLoop()
	{
		while (!bExitRequested)
		{
			GameThreadTick();
			Window::GetMain()->PollWindowEventsAndErrors();
		}

		End();
	}

	void Engine::GameThreadTick()
	{
		TURBO_LOG(LOG_ENGINE, LOG_DISPLAY, "Engine Tick");
	}

	void Engine::HandleMainWindowEvents(EWindowEvent Event)
	{
		if (Event == EWindowEvent::WindowCloseRequest)
		{
			RequestExit(EExitCode::Success);
		}
	}

	void Engine::End()
	{
		TURBO_LOG(LOG_ENGINE, LOG_INFO, "Begin exit sequence.");

		VulkanRHI::Destroy();
		Window::DestroyMainWindow();
	}

	void Engine::RequestExit(EExitCode InExitCode)
	{
		bExitRequested = true;
		ExitCode = InExitCode;
	}
} // Turbo
