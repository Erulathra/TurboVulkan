#include "Core/Engine.h"

#include "Assets/AssetManager.h"
#include "Core/CoreTimer.h"
#include "Core/Window.h"
#include "Core/Input/Input.h"
#include "Core/Input/FSDLInputSystem.h"
#include "Core/Input/Keys.h"
#include "Graphics/GeometryBuffer.h"
#include "Graphics/GPUDevice.h"
#include "Services/ImGUIService.h"
#include "Services/ILayer.h"
#include "World/World.h"

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

#if 0
		// TODO: Move thread configuration to separate class
		pthread_setname_np(pthread_self(), "Turbo Engine");
#endif

		entt::locator<FLayersStack>::emplace();
		gEngine->RegisterEngineLayers();
	}

	int32_t FEngine::Start(int32 argc, char* argv[])
	{
		// TODO: Move me elsewhere
		spdlog::set_level(spdlog::level::debug);

		mEngineState = EEngineState::Initializing;

		entt::locator<FCoreTimer>::reset(new FCoreTimer());
		FCoreTimer& coreTimer = entt::locator<FCoreTimer>::value();
		coreTimer.Init();

		entt::locator<FGPUDevice>::reset(new FGPUDevice());
		FGPUDevice& gpu = entt::locator<FGPUDevice>::value();

		entt::locator<FWindow>::reset(new FWindow());
		FWindow& window = entt::locator<FWindow>::value();

		entt::locator<IInputSystem>::reset<FSDLInputSystem>(new FSDLInputSystem());

		window.InitBackend();

		FGPUDeviceBuilder gpuDeviceBuilder;
		gpu.Init(gpuDeviceBuilder);

		FGeometryBuffer& geometryBuffer = entt::locator<FGeometryBuffer>::emplace(&gpu);
		geometryBuffer.Init(window.GetFrameBufferSize());

		window.OnWindowEvent.AddRaw(this, &ThisClass::HandleMainWindowEvents);

		IInputSystem& inputSystem = entt::locator<IInputSystem>::value();
		inputSystem.Init();
		SetupBasicInputBindings();

		entt::locator<FAssetManager>::emplace<FAssetManager>();

		// TODO: this is a bad place to initialize the world.
		mWorld = std::make_unique<FWorld>();
		mWorld->InitSceneGraph();

		for (const TSharedPtr<ILayer>& layer : entt::locator<FLayersStack>::value())
		{
			layer->Start();
		}

		window.ShowWindow(true);

		mEngineState = EEngineState::Running;

		GameThreadLoop();

		mEngineState = EEngineState::Finalizing;
		End();

		return static_cast<int32_t>(mExitCode);
	}

	void FEngine::GameThreadLoop()
	{
		FWindow& window = entt::locator<FWindow>::value();
		while (!mbExitRequested)
		{
			GameThreadTick();
			window.PollWindowEventsAndErrors();
		}
	}

	void FEngine::GameThreadTick()
	{
		TRACE_ZONE_SCOPED_N("GameThreadTick")

		FCoreTimer& coreTimer = entt::locator<FCoreTimer>::value();
		coreTimer.Tick();
		const double deltaTime = coreTimer.GetDeltaTime();

		FLayersStack& layerStack = entt::locator<FLayersStack>::value();
		{
			TRACE_ZONE_SCOPED_N("Services: Begin Tick")
			for (const TSharedPtr<ILayer>& layer : layerStack)
			{
				if (layer->ShouldTick())
				{
					layer->BeginTick_GameThread(deltaTime);
				}
			}
		}

		{
			TRACE_ZONE_SCOPED_N("Services: End Tick")
			for (auto layerIt = layerStack.rbegin(); layerIt != layerStack.rend(); ++layerIt)
			{
				if (ILayer* layer = layerIt->get();
					layer->ShouldTick())
				{
					layer->EndTick_GameThread(deltaTime);
				}
			}
		}

		FGPUDevice& gpu = entt::locator<FGPUDevice>::value();

		if (gpu.BeginFrame())
		{
			FGeometryBuffer& geometryBuffer = entt::locator<FGeometryBuffer>::value();

			FCommandBuffer* cmd = gpu.GetCommandBuffer();
			cmd->ClearImage(geometryBuffer.GetColor());

			{
				TRACE_ZONE_SCOPED_N("Services: Post begin frame")
				for (const TSharedPtr<ILayer>& layer : layerStack)
				{
					if (layer->ShouldRender())
					{
						layer->PostBeginFrame_RenderThread(&gpu, cmd);
					}
				}
			}

			geometryBuffer.BlitResultToTexture(cmd, gpu.GetPresentImage());
			THandle<FTexture> presentImage = gpu.GetPresentImage();

			{
				TRACE_ZONE_SCOPED_N("Services: Begin presenting frame")
				for (auto layerIt = layerStack.rbegin(); layerIt != layerStack.rend(); ++layerIt)
				{
					if (ILayer* layer = layerIt->get();
						layer->ShouldRender())
					{
						layer->BeginPresentingFrame_RenderThread(&gpu, cmd, presentImage);
					}
				}
			}

			gpu.PresentFrame();
		}

		TRACE_MARK_FRAME();
	}

	void FEngine::RegisterEngineLayers()
	{
		FLayersStack& layerStack = entt::locator<FLayersStack>::value();
		layerStack.PushLayer<FImGuiLayer>();
	}

	void FEngine::SetupBasicInputBindings()
	{
		const FName ToggleFullscreenName("ToggleFullscreen");
		IInputSystem& inputSystem = entt::locator<IInputSystem>::value();
		inputSystem.RegisterBinding(ToggleFullscreenName, EKeys::F11);
		inputSystem.GetActionEvent(ToggleFullscreenName).AddLambda(
			[](const FActionEvent& actionEvent)
			{
				if (actionEvent.bDown)
				{
					FWindow& window = entt::locator<FWindow>::value();
					window.SetFullscreen(!window.IsFullscreenEnabled());
				}
			});
	}

	void FEngine::HandleMainWindowEvents(EWindowEvent event)
	{
		FWindow& window = entt::locator<FWindow>::value();

		if (event == EWindowEvent::WindowCloseRequest)
		{
			RequestExit(EExitCode::Success);
		}
		else if (event == EWindowEvent::WindowResized)
		{
			const glm::vec2 windowSize = window.GetFrameBufferSize();
			TURBO_LOG(LOG_ENGINE, Info, "Window resized. New size {}", windowSize)
		}
	}

	void FEngine::End()
	{
		TURBO_LOG(LOG_ENGINE, Info, "Begin exit sequence.");

		FGPUDevice& gpu = entt::locator<FGPUDevice>::value();
		gpu.WaitIdle();

		FLayersStack& layerStack = entt::locator<FLayersStack>::value();
		for (auto layerIt = layerStack.rbegin(); layerIt != layerStack.rend(); ++layerIt)
		{
			layerIt->get()->Shutdown();
		}

		entt::locator<FGeometryBuffer>::value().Destroy();
		gpu.Shutdown();
		entt::locator<FGeometryBuffer>::reset();

		entt::locator<IInputSystem>::value().Destroy();
		entt::locator<IInputSystem>::reset();

		FWindow& window = entt::locator<FWindow>::value();
		window.Destroy();
		window.StopBackend();

		entt::locator<FWindow>::reset();
		entt::locator<FGPUDevice>::reset();
	}

	void FEngine::RequestExit(EExitCode InExitCode)
	{
		mbExitRequested = true;
		mExitCode = InExitCode;
	}
} // Turbo
