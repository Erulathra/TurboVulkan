#include "Core/Engine.h"

#include "Core/RHI/VulkanRHI.h"
#include "Core/Window.h"

namespace Turbo
{
	FEngine::FEngine()
		: mbExitRequested(false)
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

		mRHIInstance = std::unique_ptr<FVulkanRHI>(new FVulkanRHI());
		mMainWindowInstance = std::unique_ptr<FSDLWindow>(new FSDLWindow());

		mMainWindowInstance->InitBackend();
		mRHIInstance->InitWindow(mMainWindowInstance.get());

		if (!mMainWindowInstance->Init())
		{
			return static_cast<int32_t>(EExitCode::WindowCreationError);
		}

		mMainWindowInstance->OnWindowEvent.append(
			[this](EWindowEvent WindowEvent)
			{
				HandleMainWindowEvents(WindowEvent);
			});

		mRHIInstance->Init();
		mMainWindowInstance->ShowWindow(true);

		GameThreadLoop();

		return static_cast<int32_t>(mExitCode);
	}

	void FEngine::GameThreadLoop()
	{
		while (!mbExitRequested)
		{
			GameThreadTick();
			mMainWindowInstance->PollWindowEventsAndErrors();
		}

		End();
	}

	void FEngine::GameThreadTick()
	{
		TURBO_LOG(LOG_ENGINE, LOG_DISPLAY, "Engine Tick");
	}

	void FEngine::HandleMainWindowEvents(EWindowEvent event)
	{
		if (event == EWindowEvent::WindowCloseRequest)
		{
			RequestExit(EExitCode::Success);
		}
	}

	void FEngine::End()
	{
		TURBO_LOG(LOG_ENGINE, LOG_INFO, "Begin exit sequence.");

		mRHIInstance->Destroy();
		mMainWindowInstance->Destroy();
		mMainWindowInstance->StopBackend();

		mRHIInstance.release();
		mMainWindowInstance.release();
	}

	void FEngine::RequestExit(EExitCode InExitCode)
	{
		mbExitRequested = true;
		mExitCode = InExitCode;
	}
} // Turbo
