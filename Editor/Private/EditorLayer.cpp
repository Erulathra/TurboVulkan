#include "EditorLayer.h"

#include "imgui.h"
#include "Core/Engine.h"
#include "Core/Window.h"
#include "Core/WindowEvents.h"
#include "Core/Input/Input.h"
#include "Core/Input/Keys.h"
#include "Graphics/Debug.h"
#include "Graphics/GeometryBuffer.h"
#include "Graphics/GPUDevice.h"
#include "Graphics/FrameGraph/RenderGraphUtils.h"
#include "Layers/ImGUILayer.h"

namespace Turbo
{
	const FName kToggleFullscreenName = FName("ToggleFullscreen");
	const FName kFrameCapture = FName("FrameCapture");

	void FEditorLayer::Start()
	{
		IInputSystem& inputSystem = entt::locator<IInputSystem>::value();
		inputSystem.RegisterBinding({kToggleFullscreenName, EKeys::F11});
		inputSystem.RegisterBinding({kFrameCapture, EKeys::F12});

		mViewportWindow = MakeShared<FEditorViewportWindow>();
		mViewportWindow->Init();
		mOutlinerWindow = MakeShared<FSceneOutlinerWindow>();
	}

	void FEditorLayer::Shutdown()
	{
		mViewportWindow->Shutdown();
	}

	void FEditorLayer::BeginTick(double deltaTime)
	{
		mViewportWindow->Tick(deltaTime);
	}

	void FEditorLayer::EndTick(double deltaTime)
	{
		mViewportWindow->Draw();
		mOutlinerWindow->Draw();
	}

	bool FEditorLayer::ShouldTick()
	{
		return true;
	}

	void FEditorLayer::EndFrame(FRenderGraphBuilder& graphBuilder, FRGResourceHandle presentTexture)
	{
		FGPUDevice& gpu = entt::locator<FGPUDevice>::value();
		const uint32 bufferedFrameId = gpu.GetBufferedFrameId();

		std::vector<THandle<FTexture>>& renderedTextures = mViewportWindow->mRenderedTextures;
		if (bufferedFrameId < renderedTextures.size())
		{
			const FRGResourceHandle viewportTexture = graphBuilder.RegisterExternalTexture(renderedTextures[bufferedFrameId], ETextureLayout::Undefined);

			const FGeometryBuffer& geometryBuffer = entt::locator<FGeometryBuffer>::value();
			RenderGraphUtils::AddBlitTexturePass(graphBuilder, geometryBuffer.mColor, viewportTexture);
		}

		RenderGraphUtils::AddClearTexturePass(graphBuilder, presentTexture, ELinearColor::kBlack);
	}

	bool FEditorLayer::ShouldRender()
	{
		return true;
	}

	void FEditorLayer::OnEvent(FEventBase& event)
	{
		FEventDispatcher::DispatchLayer<FActionEvent>(event, this, &FEditorLayer::HandleInputActionEvent);
		FEventDispatcher::DispatchLayer<FCloseWindowEvent>(event, this, &FEditorLayer::HandleCloseEvent);

		mViewportWindow->HandleEvent(event);
	}

	FName FEditorLayer::GetName()
	{
		return GetStaticLayerName<FEditorLayer>();
	}

	void FEditorLayer::SetSelection(entt::entity entity)
	{
		if (mSelection != entity)
		{
			mSelection = entity;
			OnSelectionChanged.Broadcast(entity);
		}
	}

	void FEditorLayer::HandleInputActionEvent(FActionEvent& event)
	{
		if (event.mName == kToggleFullscreenName && event.mbDown)
		{
			FWindow& window = entt::locator<FWindow>::value();
			window.SetFullscreen(!window.IsFullscreenEnabled());
			event.Handle();
		}
		else if (event.mName == kFrameCapture && event.mbDown)
		{
			entt::locator<IFrameDebuggerAPI>::value().CaptureFrame();
			event.Handle();
		}
	}

	void FEditorLayer::HandleCloseEvent(FCloseWindowEvent& event)
	{
		gEngine->RequestExit(EExitCode::Success);
		event.Handle();
	}
} // Turbo