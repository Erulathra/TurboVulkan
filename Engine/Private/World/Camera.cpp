#include "World/Camera.h"

#include "Core/CoreTimer.h"
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

	void FCameraUtils::UpdateFreeCamera(entt::registry& registry, glm::float3 movementInput, glm::float2 rotationInput)
	{
		const auto view = registry.view<FFreeCamera, FCamera const, FTransform const, FMainViewport const>();
		for (entt::entity entity : view)
		{
			const FFreeCamera& camera = view.get<FFreeCamera>(entity);

			FTransform transform = view.get<FTransform>(entity);
			transform.mPosition += glm::normalize(movementInput) * camera.mMovementSpeed * static_cast<float>(FCoreTimer::DeltaTime());
			transform.mRotation *= glm::quat(glm::float3(rotationInput.y, 0.f, rotationInput.x) * camera.mRotationSpeed);

			registry.replace<FTransform>(entity, transform);
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