#include "World/Camera.h"

#include "Assets/AssetManager.h"
#include "Assets/AssetManager.h"
#include "Assets/AssetManager.h"
#include "Assets/AssetManager.h"
#include "Assets/AssetManager.h"
#include "Assets/AssetManager.h"
#include "Assets/AssetManager.h"
#include "Assets/AssetManager.h"
#include "Core/CoreTimer.h"
#include "Core/Math/FRotator.h"
#include "World/SceneGraph.h"

namespace Turbo
{
	void FCamera::on_construct(entt::registry& registry, const entt::entity entity)
	{
		registry.emplace<FCameraCache>(entity);
		registry.emplace<FCameraDirty>(entity);
	}

	void FCamera::on_update(entt::registry& registry, const entt::entity entity)
	{
		registry.emplace<FCameraDirty>(entity);
	}

	void FCameraUtils::UpdateDirtyCameras(entt::registry& registry)
	{
		TRACE_ZONE_SCOPED()

		const auto view = registry.view<FCamera const, FCameraDirty>();
		for (const entt::entity& entity : view)
		{
			const FCamera& camera = view.get<FCamera>(entity);
			TURBO_CHECK(glm::abs(camera.mAspectRatio) > TURBO_SMALL_NUMBER);

			glm::float4x4 projectionMatrix;

			switch (camera.mProjectionType)
			{
			case EProjectionType::Perspective:
				{
					projectionMatrix = glm::perspective(camera.mFov, camera.mAspectRatio, camera.mFarPlane, camera.mNearPlane);
					break;
				}
			case EProjectionType::Orthographic:
				{
					const float halfWidth = camera.mOrtoWidth * 0.5f;
					const float halfHeight = halfWidth / camera.mAspectRatio;
					projectionMatrix = glm::ortho(-halfWidth, halfWidth, -halfHeight, halfHeight);
					break;
				}
			default:
				{
					projectionMatrix = glm::float4x4(1.f);
				}
			}

			FMath::ConvertTurboToVulkanCoordinates(projectionMatrix);

			FCameraCache& cameraCache = registry.get_or_emplace<FCameraCache>(entity);
			cameraCache.mProjectionMatrix = projectionMatrix;
		}

		registry.remove<FCameraDirty>(view.begin(), view.end());
	}

	void FCameraUtils::UpdateFreeCameraPosition(entt::registry& registry, const glm::float3& movementInput, float deltaTime)
	{
		if (glm::length2(movementInput) < TURBO_SMALL_NUMBER)
		{
			return;
		}

		const auto view = registry.view<FFreeCamera, FCamera const, FTransform const, FMainViewport const>();
		for (entt::entity entity : view)
		{
			const FFreeCamera& camera = view.get<FFreeCamera>(entity);

			const glm::float3 normalizedInput = glm::normalize(movementInput);

			const glm::float3 rightMovement = camera.mRotator.Right() * normalizedInput.x;
			const glm::float3 upMovement = EFloat3::Up * normalizedInput.y;
			const glm::float3 forwardMovement = camera.mRotator.Forward() * normalizedInput.z;
			// const glm::float3 forwardMovement = EFloat3::Forward * normalizedInput.z;

			const glm::float3 deltaPos = (rightMovement + upMovement + forwardMovement) * camera.mMovementSpeed * deltaTime;

			FTransform transform = view.get<FTransform>(entity);
			transform.mPosition += deltaPos;
			registry.replace<FTransform>(entity, transform);
		}
	}

	void FCameraUtils::UpdateFreeCameraRotation(entt::registry& registry, const glm::float2& deltaRotation)
	{
		if (glm::length2(deltaRotation) < TURBO_SMALL_NUMBER)
		{
			return;
		}

		const auto view = registry.view<FFreeCamera, FCamera const, FTransform const, FMainViewport const>();
		for (entt::entity entity : view)
		{
			FFreeCamera& camera = view.get<FFreeCamera>(entity);
			camera.mRotator += FRotator(deltaRotation.y, deltaRotation.x, 0.f) * camera.mRotationSensitivity;
			camera.mRotator = camera.mRotator.Normalize();

			FTransform transform = view.get<FTransform>(entity);
			transform.mRotation = camera.mRotator.ToQuat();

			registry.replace<FTransform>(entity, transform);
		}
	}

	void FCameraUtils::UpdateFreeCameraSpeed(entt::registry& registry, const int32 deltaSpeed)
	{
		if (deltaSpeed == 0)
		{
			return;
		}

		const auto view = registry.view<FFreeCamera, FCamera const, FTransform const, FMainViewport const>();
		for (entt::entity entity : view)
		{
			FFreeCamera& camera = view.get<FFreeCamera>(entity);
			if (deltaSpeed > 0)
			{
				camera.mMovementSpeed *= camera.mMovementSpeedFactor;
			}
			else
			{
				camera.mMovementSpeed /= camera.mMovementSpeedFactor;
			}

			camera.mMovementSpeed = glm::clamp(camera.mMovementSpeed, camera.mMinMovementSpeed, camera.mMaxMovementSpeed);
		}
	}

	void FCameraUtils::InitializeCamera(entt::registry& registry, entt::entity entity)
	{
		if (!registry.all_of<FTransform>(entity))
		{
			registry.emplace<FTransform>(entity);
		}

		if (!registry.all_of<FCamera>(entity))
		{
			registry.emplace<FCamera>(entity);
		}

		if (!registry.all_of<FRelationship>(entity))
		{
			registry.emplace<FRelationship>(entity);
		}
	}

	void FCameraUtils::InitializeFreeCamera(entt::registry& registry, entt::entity entity)
	{
		InitializeCamera(registry, entity);
		if (!registry.all_of<FFreeCamera>(entity))
		{
			registry.emplace<FFreeCamera>(entity);
		}
	}
} // Turbo