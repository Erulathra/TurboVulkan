#include "Core/Engine.h"

#include "Core/CoreTimer.h"
#include "Core/RHI/VulkanRHI.h"
#include "Core/Window.h"
#include "Core/Input/Input.h"
#include "Core/Input/FSDLInputSystem.h"
#include "Core/Input/Keys.h"

namespace Turbo
{
	FEngine::FEngine()
		: mbExitRequested(false)
	{
	}

	FEngine::~FEngine() = default;

	void FEngine::Init()
	{
		TURBO_LOG(LOG_ENGINE, Info, "Creating engine instance.");
		gEngine = std::unique_ptr<FEngine>(new FEngine());

		// TODO: Move thread configuration to separate class
		pthread_setname_np(pthread_self(), "Turbo Engine");
	}

	int32_t FEngine::Start(int32 argc, char* argv[])
	{
		// TODO: Move me elsewhere
		spdlog::set_level(spdlog::level::debug);

		mEngineState = EEngineState::Initializing;

		mCoreTimer = std::unique_ptr<FCoreTimer>(new FCoreTimer());
		mCoreTimer->Init();

		mRHIInstance = std::unique_ptr<FVulkanRHI>(new FVulkanRHI());
		mMainWindowInstance = std::unique_ptr<FSDLWindow>(new FSDLWindow());
		mInputSystemInstance = std::unique_ptr<FSDLInputSystem>(new FSDLInputSystem());

		mMainWindowInstance->InitBackend();
		mRHIInstance->InitWindow(mMainWindowInstance.get());

		if (!mMainWindowInstance->Init())
		{
			return static_cast<int32_t>(EExitCode::WindowCreationError);
		}
		mMainWindowInstance->OnWindowEvent.AddRaw(this, &ThisClass::HandleMainWindowEvents);

		mInputSystemInstance->Init();

		SetupBasicInputBindings();

		mRHIInstance->Init();
		mMainWindowInstance->ShowWindow(true);

		mEngineState = EEngineState::Running;

		GameThreadLoop();

		mEngineState = EEngineState::Finalizing;
		End();

		return static_cast<int32_t>(mExitCode);
	}

	void FEngine::GameThreadLoop()
	{
		while (!mbExitRequested)
		{
			GameThreadTick();
			mMainWindowInstance->PollWindowEventsAndErrors();
		}
	}

	void FEngine::GameThreadTick()
	{
		mCoreTimer->Tick();

		TURBO_LOG(LOG_ENGINE, Display, "Engine Tick. FrameTime: {}, FPS: {}", mCoreTimer->GetDeltaTime(), 1.f / mCoreTimer->GetDeltaTime());

		GetRHI()->Tick();

		TRACE_MARK_FRAME();
	}

	void FEngine::SetupBasicInputBindings()
	{
		const FName ToggleFullscreenName("ToggleFullscreen");
		mInputSystemInstance->RegisterBinding(ToggleFullscreenName, EKeys::F12);
		mInputSystemInstance->GetActionEvent(ToggleFullscreenName).AddLambda([this](const FActionEvent& actionEvent)
		{
			if (actionEvent.bDown)
			{
				mMainWindowInstance->SetFullscreen(!mMainWindowInstance->IsFullscreenEnabled());
			}
		});
	}

	void FEngine::HandleMainWindowEvents(EWindowEvent event)
	{
		if (event == EWindowEvent::WindowCloseRequest)
		{
			RequestExit(EExitCode::Success);
		}
		else if (event == EWindowEvent::WindowResized)
		{
			const glm::vec2 windowSize = GetWindow()->GetFrameBufferSize();
			TURBO_LOG(LOG_ENGINE, Info, "Window resized. New size {}", windowSize)
		}
	}

	void FEngine::End()
	{
		TURBO_LOG(LOG_ENGINE, Info, "Begin exit sequence.");

		mRHIInstance->Destroy();
		mInputSystemInstance->Destroy();
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
