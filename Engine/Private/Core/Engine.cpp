#include "Core/Engine.h"

#include "Core/RHI/VulkanRHI.h"
#include "Core/Window.h"

namespace Turbo
{
	FEngine::FEngine()
		: bExitRequested(false)
	{
	}

	FEngine::~FEngine() = default;

	void FEngine::Init()
	{
		TURBO_LOG(LOG_ENGINE, LOG_INFO, "Creating engine instance.");
		gEngine = std::unique_ptr<FEngine>(new FEngine());

		// TODO: Move thread configuration to separate class
		pthread_setname_np(pthread_self(), "GameThread");
	}

	int32_t FEngine::Start(int32 argc, char* argv[])
	{
		// TODO: Move me elsewhere
		spdlog::set_level(spdlog::level::debug);

		RHIInstance = std::unique_ptr<FVulkanRHI>(new FVulkanRHI());
		MainWindowInstance = std::unique_ptr<FSDLWindow>(new FSDLWindow());

		MainWindowInstance->InitBackend();
		RHIInstance->InitWindow(MainWindowInstance.get());

		if (!MainWindowInstance->Init())
		{
			return static_cast<int32_t>(EExitCode::WindowCreationError);
		}

		MainWindowInstance->OnWindowEvent.append(
			[this](EWindowEvent WindowEvent)
			{
				HandleMainWindowEvents(WindowEvent);
			});

		RHIInstance->Init();
		MainWindowInstance->ShowWindow(true);

		GameThreadLoop();

		return static_cast<int32_t>(ExitCode);
	}

	void FEngine::GameThreadLoop()
	{
		while (!bExitRequested)
		{
			GameThreadTick();
			MainWindowInstance->PollWindowEventsAndErrors();
		}

		End();
	}

	void FEngine::GameThreadTick()
	{
		TURBO_LOG(LOG_ENGINE, LOG_DISPLAY, "Engine Tick");
	}

	void FEngine::HandleMainWindowEvents(EWindowEvent Event)
	{
		if (Event == EWindowEvent::WindowCloseRequest)
		{
			RequestExit(EExitCode::Success);
		}
	}

	void FEngine::End()
	{
		TURBO_LOG(LOG_ENGINE, LOG_INFO, "Begin exit sequence.");

		RHIInstance->Destroy();
		MainWindowInstance->Destroy();
		MainWindowInstance->StopBackend();

		RHIInstance.release();
		MainWindowInstance.release();
	}

	void FEngine::RequestExit(EExitCode InExitCode)
	{
		bExitRequested = true;
		ExitCode = InExitCode;
	}
} // Turbo
