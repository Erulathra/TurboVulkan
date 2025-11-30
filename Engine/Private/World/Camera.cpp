#include "World/Camera.h"

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
					projectionMatrix = glm::perspective(camera.mFov, camera.mAspectRatio, camera.mNearPlane, camera.mFarPlane);
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
} // Turbo