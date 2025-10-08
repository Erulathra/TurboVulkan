#include "Core/Engine.h"

#include "Core/CoreTimer.h"
#include "Core/Window.h"
#include "Core/Input/Input.h"
#include "Core/Input/FSDLInputSystem.h"
#include "Core/Input/Keys.h"
#include "Graphics/GeometryBuffer.h"
#include "Graphics/GPUDevice.h"
#include "Graphics/GraphicsLocator.h"
#include "Services/ImGUIService.h"
#include "Services/ILayer.h"

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
		gEngine = TUniquePtr<FEngine>(new FEngine());

		// TODO: Move thread configuration to separate class
		pthread_setname_np(pthread_self(), "Turbo Engine");

		gEngine->RegisterEngineLayers();
	}

	int32_t FEngine::Start(int32 argc, char* argv[])
	{
		// TODO: Move me elsewhere
		spdlog::set_level(spdlog::level::debug);

		mEngineState = EEngineState::Initializing;

		mCoreTimer = TSharedPtr<FCoreTimer>(new FCoreTimer());
		mCoreTimer->Init();

		mGpuDevice = TSharedPtr<FGPUDevice>(new FGPUDevice());
		mWindow = TSharedPtr<FWindow>(new FWindow());
		mInputSystemInstance = TUniquePtr<FSDLInputSystem>(new FSDLInputSystem());

		mWindow->InitBackend();

		FGPUDeviceBuilder gpuDeviceBuilder;
		gpuDeviceBuilder.SetWindow(mWindow);

		mGpuDevice->Init(gpuDeviceBuilder);
		FGraphicsLocator::InitGeometryBuffer(mGpuDevice.get());
		FGraphicsLocator::GetGeometryBuffer().Init(mWindow->GetFrameBufferSize());

		mWindow->OnWindowEvent.AddRaw(this, &ThisClass::HandleMainWindowEvents);

		mInputSystemInstance->Init();
		SetupBasicInputBindings();

		for (const TSharedPtr<ILayer>& layer : *FLayersStack::Get())
		{
			layer->Start();
		}

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
		TRACE_ZONE_SCOPED_N("GameThreadTick")
		mCoreTimer->Tick();
		const float deltaTime = mCoreTimer->GetDeltaTime();

		TURBO_LOG(LOG_ENGINE, Display, "Engine Tick. FrameTime: {}, FPS: {}", mCoreTimer->GetDeltaTime(), 1.f / mCoreTimer->GetDeltaTime());

		FLayersStack* layerStack = FLayersStack::Get();
		{
			TRACE_ZONE_SCOPED_N("Services: Begin Tick")
			for (const TSharedPtr<ILayer>& layer : *layerStack)
			{
				if (layer->ShouldTick())
				{
					layer->BeginTick_GameThread(deltaTime);
				}
			}
		}

		{
			TRACE_ZONE_SCOPED_N("Services: End Tick")
			for (auto layerIt = layerStack->rbegin(); layerIt != layerStack->rend(); ++layerIt)
			{
				if (ILayer* layer = layerIt->get();
					layer->ShouldTick())
				{
					layer->EndTick_GameThread(deltaTime);
				}
			}
		}

		FGPUDevice* gpu = GetGpu();
		TURBO_CHECK(gpu);

		if (gpu->BeginFrame())
		{
			FCommandBuffer* cmd = gpu->GetCommandBuffer();
			{
				TRACE_ZONE_SCOPED_N("Services: Post begin frame")
				for (const TSharedPtr<ILayer>& layer : *layerStack)
				{
					if (layer->ShouldRender())
					{
						layer->PostBeginFrame_RenderThread(gpu, cmd);
					}
				}
			}

			FGraphicsLocator::GetGeometryBuffer().BlitResultToTexture(cmd, gpu->GetPresentImage());
			FTextureHandle presentImage = gpu->GetPresentImage();

			{
				TRACE_ZONE_SCOPED_N("Services: Begin presenting frame")
				for (auto layerIt = layerStack->rbegin(); layerIt != layerStack->rend(); ++layerIt)
				{
					if (ILayer* layer = layerIt->get();
						layer->ShouldRender())
					{
						layer->BeginPresentingFrame_RenderThread(gpu, cmd, presentImage);
					}
				}
			}

			gpu->PresentFrame();
		}

		TRACE_MARK_FRAME();
	}

	void FEngine::RegisterEngineLayers()
	{
		FLayersStack* layerStack = FLayersStack::Get();
		layerStack->PushLayer<FImGuiLayer>();
	}

	void FEngine::SetupBasicInputBindings()
	{
		const FName ToggleFullscreenName("ToggleFullscreen");
		mInputSystemInstance->RegisterBinding(ToggleFullscreenName, EKeys::F11);
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

		mGpuDevice->WaitIdle();

		FLayersStack* layerStack = FLayersStack::Get();
		for (auto layerIt = layerStack->rbegin(); layerIt != layerStack->rend(); ++layerIt)
		{
			layerIt->get()->Shutdown();
		}

		FGraphicsLocator::GetGeometryBuffer().Destroy();

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
