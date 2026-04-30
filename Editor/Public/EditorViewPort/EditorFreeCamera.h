#pragma once

#include "Core/Input/Input.h"
#include "Core/Input/Keys.h"

namespace Turbo
{
	struct FActionEvent;
	struct FEventBase;

	struct FEditorFreeCameraUtils
	{
		static void Init();
		static void RegisterEvents();
		static void HandleEvent(FEventBase& Event, bool bViewportFocused);
		static void Tick(double deltaTime);

	private:
		static void OnConstructMainViewPort(entt::registry& registry, const entt::entity& entity);
		static void OnDestroyMainViewPort(entt::registry& registry, const entt::entity& entity);

		static void HandleAction(FActionEvent& actionEvent, bool bViewportFocused = true);
		static bool HandleEnableAction(FActionEvent& actionEvent, bool bViewportFocused = true);
		static bool HandleMovementAction(FActionEvent& actionEvent);
		static bool HandleRotationAction(FActionEvent& actionEvent);
		static bool HandleChangeSpeedAction(FActionEvent& actionEvent);
	};
} // Turbo