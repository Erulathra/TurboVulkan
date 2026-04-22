#pragma once
#include "Core/Input/Input.h"

namespace Turbo
{
	struct FEventBase;

	struct FFlyMovementComp
	{
		glm::float3 mMoveInputValue = glm::float3(0.f);
	};

	struct FFlyMovementSystem
	{
		static void Enable();
		static void Disable();

		static void HandleEvent(FEventBase& Event);
		static void Tick(double deltaTime);

		// Input Actions
		static void HandleAction(FActionEvent& actionEvent);
		static bool HandleMovement(FActionEvent& actionEvent);
		static bool HandleRotation(FActionEvent& actionEvent);
	};
} // Turbo
