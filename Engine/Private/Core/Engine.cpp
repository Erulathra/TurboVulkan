#include "Core/Engine.h"

#include "TaskScheduler.h"
#include "Assets/AssetManager.h"
#include "Assets/EngineResources.h"
#include "Assets/MaterialManager.h"
#include "Core/CoreTimer.h"
#include "Core/Window.h"
#include "Core/WindowEvents.h"
#include "Core/Input/FSDLInputSystem.h"
#include "Core/Input/Input.h"
#include "Core/Math/Random.h"
#include "Graphics/GeometryBuffer.h"
#include "Graphics/GPUDevice.h"
#include "Layers/ImGUILayer.h"
#include "Layers/Layer.h"
#include "Layers/SceneRenderingLayer.h"
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
		InitLogger();

		Random::SetRandomSeed();

		// TURBO_LOG(LOG_ENGINE, Info, "Creating engine instance.");
		TURBO_LOG(LogEngine, Info, "Creating engine instance.")
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
		mEngineState = EEngineState::Initializing;

		entt::locator<FCoreTimer>::reset(new FCoreTimer());
		FCoreTimer& coreTimer = entt::locator<FCoreTimer>::value();
		coreTimer.Init();

		// Additional thread for io tasks
		enki::TaskSchedulerConfig taskSchedulerConfig;

		entt::locator<enki::TaskScheduler>::emplace();
		enki::TaskScheduler& taskScheduler = entt::locator<enki::TaskScheduler>::value();
		taskScheduler.Initialize(taskSchedulerConfig);

		entt::locator<FGPUDevice>::reset(new FGPUDevice());
		FGPUDevice& gpu = entt::locator<FGPUDevice>::value();

		entt::locator<FWindow>::reset(new FWindow());
		FWindow& window = entt::locator<FWindow>::value();

		entt::locator<IInputSystem>::reset<FSDLInputSystem>(new FSDLInputSystem());

		window.InitBackend();

		FGPUDeviceBuilder gpuDeviceBuilder;
		gpu.Init(gpuDeviceBuilder);

		FAssetManager& assetManager = entt::locator<FAssetManager>::emplace<FAssetManager>();
		assetManager.Init(gpu);

		FMaterialManager& materialManager = entt::locator<FMaterialManager>::emplace<FMaterialManager>();
		materialManager.Init(gpu);

		EngineMaterials::InitEngineMaterials();

		FGeometryBuffer& geometryBuffer = entt::locator<FGeometryBuffer>::emplace(&gpu);
		geometryBuffer.Init(window.GetFrameBufferSize());

		IInputSystem& inputSystem = entt::locator<IInputSystem>::value();
		inputSystem.Init();

		// TODO: this is a bad place to initialize the world.
		mWorld = std::make_unique<FWorld>();
		FSceneGraph::InitSceneGraph(mWorld->mRegistry);

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

	EEventReply FEngine::PushEvent(FEventBase& event)
	{
		OnEvent(event);

		FLayersStack& layerStack = entt::locator<FLayersStack>::value();
		for (auto It = layerStack.rbegin();  It != layerStack.rend(); ++It)
		{
			It->get()->OnEvent(event);
			if (event.mEventReply != EEventReply::Unhandled)
			{
				return event.mEventReply;
			}
		}

		return event.mEventReply;
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
					layer->BeginTick(deltaTime);
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
					layer->EndTick(deltaTime);
				}
			}
		}

		FGPUDevice& gpu = entt::locator<FGPUDevice>::value();

		if (gpu.BeginFrame())
		{
			FGeometryBuffer& geometryBuffer = entt::locator<FGeometryBuffer>::value();

			FCommandBuffer& cmd = gpu.GetMainCommandBuffer();

			const THandle<FTexture>& colorTexture = geometryBuffer.GetColor();
			cmd.TransitionImage(colorTexture, vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral);
			cmd.ClearImage(geometryBuffer.GetColor());

			cmd.TransitionImage(colorTexture, vk::ImageLayout::eGeneral, vk::ImageLayout::eColorAttachmentOptimal);
			cmd.TransitionImage(geometryBuffer.GetDepth(), vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthAttachmentOptimal);

			{
				TRACE_ZONE_SCOPED_N("Services: Post begin frame")
				for (const TSharedPtr<ILayer>& layer : layerStack)
				{
					if (layer->ShouldRender())
					{
						layer->PostBeginFrame(gpu, cmd);
					}
				}
			}

			{
				TRACE_ZONE_SCOPED_N("Services: Post begin frame")
				for (const TSharedPtr<ILayer>& layer : layerStack)
				{
					if (layer->ShouldRender())
					{
						layer->RenderScene(gpu, cmd);
					}
				}
			}

			const THandle<FTexture> presentImage = gpu.GetPresentImage();
			cmd.TransitionImage(presentImage, vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral);
			geometryBuffer.BlitResultToTexture(cmd, presentImage);

			{
				TRACE_ZONE_SCOPED_N("Services: Begin presenting frame")
				for (auto layerIt = layerStack.rbegin(); layerIt != layerStack.rend(); ++layerIt)
				{
					if (ILayer* layer = layerIt->get();
						layer->ShouldRender())
					{
						layer->BeginPresentingFrame(gpu, cmd, presentImage);
					}
				}
			}

			cmd.TransitionImage(presentImage, vk::ImageLayout::eGeneral, vk::ImageLayout::ePresentSrcKHR);
			gpu.PresentFrame();
		}

		TRACE_MARK_FRAME();
	}

	void FEngine::OnEvent(FEventBase& event)
	{
		FEventDispatcher::Dispatch<FResizeWindowEvent>(
			event, [](const FResizeWindowEvent& resizeWindowEvent)
		{
			TURBO_LOG(LogEngine, Info, "Window resized. New size {}", resizeWindowEvent.mNewWindowSize)

			FGPUDevice& gpu = entt::locator<FGPUDevice>::value();
			gpu.RequestSwapChainResize();
		});
	}

	void FEngine::RegisterEngineLayers()
	{
		FLayersStack& layerStack = entt::locator<FLayersStack>::value();
		layerStack.PushLayer<FImGuiLayer>();
		layerStack.PushLayer<FSceneRenderingLayer>();
	}

	void FEngine::End()
	{
		TURBO_LOG(LogEngine, Info, "Begin exit sequence.");

		entt::locator<enki::TaskScheduler>::value().WaitforAllAndShutdown();
		entt::locator<enki::TaskScheduler>::reset();

		FGPUDevice& gpu = entt::locator<FGPUDevice>::value();
		gpu.WaitIdle();

		FLayersStack& layerStack = entt::locator<FLayersStack>::value();
		for (auto layerIt = layerStack.rbegin(); layerIt != layerStack.rend(); ++layerIt)
		{
			layerIt->get()->Shutdown();
		}

		mWorld->UnloadLevel();

		EngineResources::DestroyEngineResources();

		entt::locator<FGeometryBuffer>::value().Destroy();
		entt::locator<FAssetManager>::value().Destroy(gpu);
		entt::locator<FMaterialManager>::value().Destroy(gpu);

		gpu.Shutdown();
		entt::locator<FGeometryBuffer>::reset();

		entt::locator<IInputSystem>::value().Destroy();
		entt::locator<IInputSystem>::reset();

		FWindow& window = entt::locator<FWindow>::value();
		window.Destroy();
		window.StopBackend();

		entt::locator<FWindow>::reset();
		entt::locator<FGPUDevice>::reset();

		entt::locator<enki::TaskScheduler>::reset();
	}

	void FEngine::RequestExit(EExitCode InExitCode)
	{
		mbExitRequested = true;
		mExitCode = InExitCode;
	}
} // Turbo
