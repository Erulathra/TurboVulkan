#include "EditorLayer.h"

#include "Core/Engine.h"
#include "Core/Window.h"
#include "Core/WindowEvents.h"
#include "Core/Input/Input.h"
#include "Core/Input/Keys.h"

namespace Turbo
{
	const FName kToggleFullscreenName = FName("ToggleFullscreen");
	const FName kExitName = FName("Exit");

	void FEditorLayer::Start()
	{
		IInputSystem& inputSystem = entt::locator<IInputSystem>::value();
		inputSystem.RegisterBinding(kToggleFullscreenName, EKeys::F11);
		inputSystem.RegisterBinding(kExitName, EKeys::Escape);
	}

	void FEditorLayer::Shutdown()
	{
	}

	void FEditorLayer::OnEvent(FEventBase& event)
	{
		FEventDispatcher::Dispatch<FActionEvent>(event, this, &FEditorLayer::HandleInputActionEvent);
		FEventDispatcher::Dispatch<FCloseWindowEvent>(event, this, &FEditorLayer::HandleCloseEvent);
	}

	FName FEditorLayer::GetName()
	{
		static FName layerName = FName("EditorLayer");
		return layerName;
	}

	void FEditorLayer::HandleInputActionEvent(FActionEvent& event)
	{
		if (event.ActionName == kToggleFullscreenName && event.bDown)
		{
			FWindow& window = entt::locator<FWindow>::value();
			window.SetFullscreen(!window.IsFullscreenEnabled());
			event.Handle();
		}
		else if (event.ActionName == kExitName && event.bDown)
		{
			gEngine->RequestExit(EExitCode::Success);
			event.Handle();
		}
	}

	void FEditorLayer::HandleCloseEvent(FCloseWindowEvent& event)
	{
		gEngine->RequestExit(EExitCode::Success);
		event.Handle();
	}
} // Turbo