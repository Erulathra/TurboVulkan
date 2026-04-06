#include "EditorLayer.h"

#include "Core/Engine.h"
#include "Core/Window.h"
#include "Core/WindowEvents.h"
#include "Core/Input/Input.h"
#include "Core/Input/Keys.h"
#include "EditorViewPort/EditorFreeCamera.h"
#include "Graphics/Debug.h"
#include "Graphics/GPUDevice.h"

namespace Turbo
{
	const FName kExitName = FName("Exit");
	const FName kFreezeRenderingName = FName("FreezeRendering");
	const FName kToggleFullscreenName = FName("ToggleFullscreen");
	const FName kFrameCapture = FName("FrameCapture");

	void FEditorLayer::Start()
	{
		IInputSystem& inputSystem = entt::locator<IInputSystem>::value();
		inputSystem.RegisterBinding({kExitName, EKeys::Escape});
		inputSystem.RegisterBinding({kFreezeRenderingName, EKeys::F10});
		inputSystem.RegisterBinding({kToggleFullscreenName, EKeys::F11});
		inputSystem.RegisterBinding({kFrameCapture, EKeys::F12});

		FEditorFreeCameraUtils::Init();
	}

	void FEditorLayer::Shutdown()
	{
	}

	void FEditorLayer::BeginTick(double deltaTime)
	{
		FEditorFreeCameraUtils::Tick(deltaTime);
	}

	bool FEditorLayer::ShouldTick()
	{
		return true;
	}

	void FEditorLayer::OnEvent(FEventBase& event)
	{
		FEventDispatcher::Dispatch<FActionEvent>(event, this, &FEditorLayer::HandleInputActionEvent);
		FEventDispatcher::Dispatch<FCloseWindowEvent>(event, this, &FEditorLayer::HandleCloseEvent);

		FEditorFreeCameraUtils::HandleEvent(event);
	}

	FName FEditorLayer::GetName()
	{
		static FName layerName = FName("EditorLayer");
		return layerName;
	}

	void FEditorLayer::HandleInputActionEvent(FActionEvent& event)
	{
		if (event.mActionName == kToggleFullscreenName && event.mbDown)
		{
			FWindow& window = entt::locator<FWindow>::value();
			window.SetFullscreen(!window.IsFullscreenEnabled());
			event.Handle();
		}
		else if (event.mActionName == kExitName && event.mbDown)
		{
			gEngine->RequestExit(EExitCode::Success);
			event.Handle();
		}
		else if (event.mActionName == kFreezeRenderingName && event.mbDown)
		{
			gFreezeRendering = !gFreezeRendering;
			event.Handle();
		}
		else if (event.mActionName == kFrameCapture && event.mbDown)
		{
			IFrameDebuggerAPI::Get()->CaptureFrame();
			event.Handle();
		}
	}

	void FEditorLayer::HandleCloseEvent(FCloseWindowEvent& event)
	{
		gEngine->RequestExit(EExitCode::Success);
		event.Handle();
	}
} // Turbo