#include "Core/Engine.h"

#include "Core/CoreTimer.h"
#include "Core/Window.h"
#include "Core/Input/Input.h"
#include "Core/Input/FSDLInputSystem.h"
#include "Core/Input/Keys.h"
#include "Graphics/GPUDevice.h"
#include "Services/IService.h"

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

		mGpuDevice = std::shared_ptr<FGPUDevice>(new FGPUDevice());
		mWindow = std::shared_ptr<FWindow>(new FWindow());
		mInputSystemInstance = std::unique_ptr<FSDLInputSystem>(new FSDLInputSystem());

		mWindow->InitBackend();

		FGPUDeviceBuilder gpuDeviceBuilder;
		gpuDeviceBuilder.SetWindow(mWindow);

		mGpuDevice->Init(gpuDeviceBuilder);
		mGpuDevice->InitGeometryBuffer();

		mWindow->OnWindowEvent.AddRaw(this, &ThisClass::HandleMainWindowEvents);

		mInputSystemInstance->Init();
		SetupBasicInputBindings();

		FServiceManager* ServiceManager = FServiceManager::Get();
		ServiceManager->ForEachService([](IService* service){ service->Start(); });

		mWindow->ShowWindow(true);

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
			mWindow->PollWindowEventsAndErrors();
		}
	}

	void FEngine::GameThreadTick()
	{
		TRACE_ZONE_SCOPED_N(GameThreadTick)
		mCoreTimer->Tick();
		const float deltaTime = mCoreTimer->GetDeltaTime();

		TURBO_LOG(LOG_ENGINE, Display, "Engine Tick. FrameTime: {}, FPS: {}", mCoreTimer->GetDeltaTime(), 1.f / mCoreTimer->GetDeltaTime());

		FServiceManager* ServiceManager = FServiceManager::Get();
		{
			TRACE_ZONE_SCOPED_N(Services: Tick)
			ServiceManager->ForEachService([deltaTime](IService* service){ service->Tick_GameThread(deltaTime); });
		}

		FGPUDevice* gpu = GetGpu();
		TURBO_CHECK(gpu);

		gpu->BeginFrame();

		FCommandBuffer* cmd = gpu->GetCommandBuffer();
		{
			TRACE_ZONE_SCOPED_N("Services: Render Frame")
			ServiceManager->ForEachService([gpu, cmd](IService* service) { service->RenderFrame_RenderThread(gpu, cmd); });
		}

		const FTextureHandle image = gpu->GetPresentImage();

		const glm::vec4 color = glm::mix(ELinearColor::kMagenta, ELinearColor::kBlue, glm::sin(FCoreTimer::TimeFromEngineStart()));
		cmd->ClearImage(image, color);

		gpu->PresentFrame();

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
				mWindow->SetFullscreen(!mWindow->IsFullscreenEnabled());
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

		FServiceManager* ServiceManager = FServiceManager::Get();
		ServiceManager->ForEachService([](IService* service){ service->Shutdown(); });

		mGpuDevice->DestroyGeometryBuffer();

		mGpuDevice->Shutdown();
		mInputSystemInstance->Destroy();
		mWindow->Destroy();
		mWindow->StopBackend();

		mGpuDevice = nullptr;
		mWindow = nullptr;
	}

	void FEngine::RequestExit(EExitCode InExitCode)
	{
		mbExitRequested = true;
		mExitCode = InExitCode;
	}
} // Turbo
