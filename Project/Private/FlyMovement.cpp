#include "FlyMovement.h"

#include "Core/Engine.h"
#include "Core/Input/Input.h"
#include "Core/Input/Keys.h"
#include "World/Camera.h"
#include "World/World.h"

namespace Turbo
{
	class IInputSystem;

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

		const std::array<const FCameraBinding, 6> kMovementBindings = {
			kMoveForward,
			kMoveBackward,
			kMoveLeft,
			kMoveRight,
			kMoveUp,
			kMoveDown,
		};

		const std::array<const FActionBinding, 2> kRotationBidings = {
			kRotateX,
			kRotateY,
		};
	}

	// Setup signals
	struct FMainViewportHandler
	{
		static void OnConstructMainViewport(entt::registry& registry, const entt::entity& entity)
		{
			registry.emplace_or_replace<FFlyMovementComp>(entity);
		}

		static void OnDestroyMainViewport(entt::registry& registry, const entt::entity& entity)
		{
			registry.emplace_or_replace<FFlyMovementComp>(entity);
		}
	};

	void FFlyMovementSystem::Enable()
	{
		// Register bindings
		IInputSystem& inputSystem = entt::locator<IInputSystem>::value();
		for (const FCameraBinding& binding : FreeCamera::kMovementBindings)
		{
			inputSystem.RegisterBinding(binding);
		}

		for (const FActionBinding& binding : FreeCamera::kRotationBidings)
		{
			inputSystem.RegisterBinding(binding);
		}

		FWorld* world = gEngine->GetWorld();
		world->mRegistry.on_construct<FMainViewport>().connect<&FMainViewportHandler::OnConstructMainViewport>();
		world->mRegistry.on_destroy<FMainViewport>().connect<&FMainViewportHandler::OnDestroyMainViewport>();
	}

	void FFlyMovementSystem::Disable()
	{
		FWorld* world = gEngine->GetWorld();
		world->mRegistry.on_construct<FMainViewport>().disconnect<&FMainViewportHandler::OnConstructMainViewport>();
		world->mRegistry.on_destroy<FMainViewport>().disconnect<&FMainViewportHandler::OnDestroyMainViewport>();
	}

	void FFlyMovementSystem::HandleEvent(FEventBase& Event)
	{
		FEventDispatcher::Dispatch<FActionEvent>(Event, &FFlyMovementSystem::HandleAction);
	}

	void FFlyMovementSystem::Tick(double deltaTime)
	{
		FWorld* world = gEngine->GetWorld();
		auto view = world->mRegistry.view<FFlyMovementComp>();

		for (const entt::entity entity : view)
		{
			const FFlyMovementComp& input = view.get<FFlyMovementComp>(entity);
			FCameraUtils::UpdateFreeCameraPosition(world->mRegistry, input.mMoveInputValue, deltaTime);
		}
	}

	void FFlyMovementSystem::HandleAction(FActionEvent& actionEvent)
	{
		if (HandleRotation(actionEvent))
		{
			return;
		}

		if (HandleMovement(actionEvent))
		{
			return;
		}
	}

	bool FFlyMovementSystem::HandleMovement(FActionEvent& actionEvent)
	{
		FWorld* world = gEngine->GetWorld();
		auto view = world->mRegistry.view<FFlyMovementComp>();

		for (const entt::entity entity : view)
		{
			for (const FCameraBinding& binding : FreeCamera::kMovementBindings)
			{
				if (binding.mActionName == actionEvent.mName)
				{
					FFlyMovementComp& input = view.get<FFlyMovementComp>(entity);

					const float directionSign = actionEvent.mbDown ? 1.f : -1.f;
					input.mMoveInputValue += binding.mDirection * directionSign;
					input.mMoveInputValue =
						glm::clamp(input.mMoveInputValue, glm::float3(-1.f), glm::float3(1.f));

					actionEvent.Handle();
					return true;
				}
			}
		}
		return false;
	}

	bool FFlyMovementSystem::HandleRotation(FActionEvent& actionEvent)
	{
		glm::float2 deltaRotation = glm::float2{0.f};

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
			return true;
		}

		return false;
	}
} // Turbo