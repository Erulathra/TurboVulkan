#include "EditorViewPort/EditorFreeCamera.h"

#include "Core/Engine.h"
#include "Layers/Event.h"
#include "Core/Input/Input.h"
#include "World/Camera.h"
#include "World/World.h"

namespace Turbo
{
	struct FEditorFreeCameraInput
	{
		bool bNavigationEnabled = false;
		glm::float3 mMoveInputValue = glm::float3(0.f);
	};

	struct FCameraBinding
	{
		FName mActionName;
		FKey mKey;
		glm::float3 mDirection;

		operator FActionBinding() const
		{
			return FActionBinding(mActionName, mKey);
		}
	};

	namespace FreeCamera
	{
		const FCameraBinding kMoveForward {FName("FreeCamera.MoveForward"), EKeys::W, EFloat3::Forward};
		const FCameraBinding kMoveBackward {FName("FreeCamera.MoveBackward"), EKeys::S, -EFloat3::Forward};
		const FCameraBinding kMoveLeft {FName("FreeCamera.MoveLeft"), EKeys::A, -EFloat3::Right};
		const FCameraBinding kMoveRight {FName("FreeCamera.MoveRight"), EKeys::D, EFloat3::Right};
		const FCameraBinding kMoveUp {FName("FreeCamera.MoveUp"), EKeys::E, EFloat3::Up};
		const FCameraBinding kMoveDown {FName("FreeCamera.MoveDown"), EKeys::Q, -EFloat3::Up};

		const FActionBinding kRotateX {FName{"FreeCamera.RotateX"}, EKeys::MouseDeltaPositionX};
		const FActionBinding kRotateY {FName{"FreeCamera.RotateY"}, EKeys::MouseDeltaPositionY};

		const FActionBinding kChangeSpeed {FName{"FreeCamera.ChangeSpeed"}, EKeys::MouseScrollY};

		const FActionBinding kEnable {FName{"FreeCamera.Enable"}, EKeys::MouseButtonRight};

		const std::array<const FCameraBinding, 6> kMovementBindings = {
			kMoveForward,
			kMoveBackward,
			kMoveLeft,
			kMoveRight,
			kMoveUp,
			kMoveDown
		};

		const std::array<const FActionBinding, 2> kRotationBidings = {
			kRotateX,
			kRotateY,
		};
	}

	void FEditorFreeCameraUtils::Init()
	{
		RegisterEvents();

		FWorld* world = gEngine->GetWorld();
		world->mRegistry.on_construct<FMainViewport>().connect<&FEditorFreeCameraUtils::OnConstructMainViewPort>();
		world->mRegistry.on_destroy<FMainViewport>().connect<&FEditorFreeCameraUtils::OnDestroyMainViewPort>();

		entt::entity cameraEntity = world->mRegistry.create();
		FCameraUtils::InitializeFreeCamera(world->mRegistry, cameraEntity);
		world->mRegistry.emplace<FMainViewport>(cameraEntity);
	}

	void FEditorFreeCameraUtils::RegisterEvents()
	{
		IInputSystem& inputSystem = entt::locator<IInputSystem>::value();
		for (const FCameraBinding& binding : FreeCamera::kMovementBindings)
		{
			inputSystem.RegisterBinding(binding);
		}

		for (const FActionBinding& binding : FreeCamera::kRotationBidings)
		{
			inputSystem.RegisterBinding(binding);
		}

		inputSystem.RegisterBinding(FreeCamera::kEnable);
		inputSystem.RegisterBinding(FreeCamera::kChangeSpeed);
	}

	void FEditorFreeCameraUtils::HandleEvent(FEventBase& Event, bool bViewportFocused)
	{
		FEventDispatcher::Dispatch<FActionEvent>(Event, &FEditorFreeCameraUtils::HandleAction, bViewportFocused);
	}

	void FEditorFreeCameraUtils::OnConstructMainViewPort(entt::registry& registry, const entt::entity& entity)
	{
		if (registry.all_of<FFreeCamera>(entity))
		{
			registry.emplace_or_replace<FEditorFreeCameraInput>(entity);
		}
	}

	void FEditorFreeCameraUtils::OnDestroyMainViewPort(entt::registry& registry, const entt::entity& entity)
	{
		registry.remove<FEditorFreeCameraInput>(entity);
	}

	void FEditorFreeCameraUtils::Tick(double deltaTime)
	{
		FWorld* world = gEngine->GetWorld();
		auto view = world->mRegistry.view<FEditorFreeCameraInput>();

		for (const entt::entity entity : view)
		{
			const FEditorFreeCameraInput& input = view.get<FEditorFreeCameraInput>(entity);

			if (input.bNavigationEnabled)
			{
				FCameraUtils::UpdateFreeCameraPosition(world->mRegistry, input.mMoveInputValue, deltaTime);
			}
		}
	}

	void FEditorFreeCameraUtils::HandleAction(FActionEvent& actionEvent, bool bViewportFocused)
	{
		if (HandleEnableAction(actionEvent, bViewportFocused))
		{
			return;
		}

		if (HandleChangeSpeedAction(actionEvent))
		{
			return;
		}

		if (HandleRotationAction(actionEvent))
		{
			return;
		}

		if (HandleMovementAction(actionEvent))
		{
			return;
		}
	}

	bool FEditorFreeCameraUtils::HandleEnableAction(FActionEvent& actionEvent, bool bViewportFocused)
	{
		FWorld* world = gEngine->GetWorld();
		auto view = world->mRegistry.view<FEditorFreeCameraInput>();

		if (actionEvent.mName == FreeCamera::kEnable.mName)
		{
			for (const entt::entity cameraEntity : view)
			{
				FEditorFreeCameraInput& freeCameraInput = view.get<FEditorFreeCameraInput>(cameraEntity);
				freeCameraInput.bNavigationEnabled = actionEvent.mbDown && bViewportFocused;

				if (!freeCameraInput.bNavigationEnabled)
				{
					freeCameraInput.mMoveInputValue = glm::float3(0.f);
				}

				FWindow& window = entt::locator<FWindow>::value();
				window.ShowCursor(!freeCameraInput.bNavigationEnabled);
			}

			return true;
		}

		return false;
	}

	bool FEditorFreeCameraUtils::HandleMovementAction(FActionEvent& actionEvent)
	{
		FWorld* world = gEngine->GetWorld();
		auto view = world->mRegistry.view<FEditorFreeCameraInput>();

		for (const entt::entity cameraEntity : view)
		{
			FEditorFreeCameraInput& freeCameraInput = view.get<FEditorFreeCameraInput>(cameraEntity);

			for (const FCameraBinding& binding : FreeCamera::kMovementBindings)
			{
				if (binding.mActionName == actionEvent.mName
					&& freeCameraInput.bNavigationEnabled)
				{
					const float directionSign = actionEvent.mbDown ? 1.f : -1.f;
					freeCameraInput.mMoveInputValue += binding.mDirection * directionSign;
					freeCameraInput.mMoveInputValue =
						glm::clamp(freeCameraInput.mMoveInputValue, glm::float3(-1.f), glm::float3(1.f));

					actionEvent.Handle();
					return true;
				}
			}
		}
		return false;
	}

	bool FEditorFreeCameraUtils::HandleRotationAction(FActionEvent& actionEvent)
	{
		glm::float2 deltaRotation = glm::float2{0.f};

		FWorld* world = gEngine->GetWorld();
		auto view = world->mRegistry.view<FEditorFreeCameraInput>();

		bool bNavigationEnabled = true;

		for (const entt::entity cameraEntity : view)
		{
			FEditorFreeCameraInput& freeCameraInput = view.get<FEditorFreeCameraInput>(cameraEntity);
			bNavigationEnabled &= freeCameraInput.bNavigationEnabled;
		}

		if (!bNavigationEnabled)
		{
			return false;
		}

		if (actionEvent.mName == FreeCamera::kRotateX.mName)
		{
			deltaRotation.x = actionEvent.mValue;
		}

		if (actionEvent.mName == FreeCamera::kRotateY.mName)
		{
			deltaRotation.y = actionEvent.mValue;
		}

		if (glm::length2(deltaRotation) > TURBO_SMALL_NUMBER)
		{
			FCameraUtils::UpdateFreeCameraRotation(gEngine->GetWorld()->mRegistry, deltaRotation);
			actionEvent.Handle();
			return true;
		}

		return false;
	}

	bool FEditorFreeCameraUtils::HandleChangeSpeedAction(FActionEvent& actionEvent)
	{
		bool bNavigationEnabled = true;

		FWorld* world = gEngine->GetWorld();
		auto view = world->mRegistry.view<FEditorFreeCameraInput>();
		for (const entt::entity cameraEntity : view)
		{
			FEditorFreeCameraInput& freeCameraInput = view.get<FEditorFreeCameraInput>(cameraEntity);
			bNavigationEnabled &= freeCameraInput.bNavigationEnabled;
		}

		if (bNavigationEnabled && actionEvent.mName == FreeCamera::kChangeSpeed.mName)
		{
			FCameraUtils::UpdateFreeCameraSpeed(world->mRegistry, glm::trunc(actionEvent.mValue));
			actionEvent.Handle();
			return true;
		}

		return false;
	}
} // Turbo