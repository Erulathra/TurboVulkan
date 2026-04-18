#include "World/Camera.h"

#include "Assets/AssetManager.h"
#include "Core/Math/FRotator.h"
#include "Graphics/GPUDevice.h"
#include "World/SceneGraph.h"

namespace Turbo
{
	inline TAutoConsoleVariable<bool> CVarFreezeCulling("culling.freeze", false, "Freezes culling");

	void FCamera::on_construct(entt::registry& registry, const entt::entity entity)
	{
		registry.emplace<FCameraCache>(entity);
		registry.emplace<FProjectionDirty>(entity);
	}

	void FCamera::on_update(entt::registry& registry, const entt::entity entity)
	{
		registry.emplace<FProjectionDirty>(entity);
	}

	void FCameraUtils::UpdateDirtyCameras(entt::registry& registry)
	{
		TRACE_ZONE_SCOPED()

		const auto view = registry.view<FCamera const, FTransform const, FProjectionDirty>();
		for (const entt::entity& entity : view)
		{
			const FCamera& camera = view.get<FCamera>(entity);
			TURBO_CHECK(glm::abs(camera.mAspectRatio) > TURBO_SMALL_NUMBER);

			glm::float4x4 projectionMatrix = {};

			switch (camera.mProjectionType)
			{
			case EProjectionType::Perspective:
				{
					const float verticalFov = 2.f * glm::atan(glm::tan(camera.mFov * 0.5f) / camera.mAspectRatio);
					projectionMatrix = glm::perspective(verticalFov, camera.mAspectRatio, camera.mFarPlane, camera.mNearPlane);
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

			FCameraCache& cameraCache = registry.get_or_emplace<FCameraCache>(entity);
			cameraCache.mProjectionMatrix = projectionMatrix;
		}

		registry.remove<FProjectionDirty>(view.begin(), view.end());
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

	void FCameraUtils::UpdateCameraFrustum(entt::registry& registry)
	{
		auto camerasView = registry.view<FCameraCache, FCamera const, FWorldTransform const, FWorldTransformDirty const, FMainViewport const>();

		for (const entt::entity entity : camerasView)
		{
			FCameraCache& cameraCache = camerasView.get<FCameraCache>(entity);
			const FCamera& camera = camerasView.get<FCamera>(entity);
			const FWorldTransform& transform = camerasView.get<FWorldTransform>(entity);

			if (CVarFreezeCulling.Get() == false)
			{
				cameraCache.mViewFrustum = GetViewFrustum(camera, transform);
			}
		}

	}

	FFrustum FCameraUtils::GetViewFrustum(const FCamera& camera, const FWorldTransform& transform)
	{
		TURBO_CHECK(glm::abs(camera.mAspectRatio) > TURBO_SMALL_NUMBER);

		FFrustum frustum;

		const glm::float3 position = TransformUtils::GetPosition(transform);

		const glm::float3 forward = TransformUtils::GetForward(transform);
		const glm::float3 up = TransformUtils::GetUp(transform);
		const glm::float3 right = glm::cross(up, forward);

		frustum.GetNear() = {forward, position + forward * camera.mNearPlane};
		frustum.GetFar() = {-forward, position + forward * camera.mFarPlane,};

		switch (camera.mProjectionType)
		{
		case EProjectionType::Perspective:
			{
                const float halfWidth = camera.mFarPlane * glm::tan(camera.mFov * 0.5f);
                const float halfHeight = halfWidth / camera.mAspectRatio;
                const glm::float3 farVector = forward * camera.mFarPlane;

                // Right, Left Plane
                const glm::float3 leftPlaneNormal = glm::cross(up, glm::normalize(farVector - right * halfWidth));
                frustum.GetLeft() = {leftPlaneNormal, position};
                const glm::float3 rightPlaneNormal = glm::cross(glm::normalize(farVector + right * halfWidth), up);
                frustum.GetRight() = {rightPlaneNormal, position};

                // Top, Bottom Plane
                const glm::float3 topPlaneNormal = glm::cross(right, glm::normalize(farVector + up * halfHeight));
                frustum.GetTop() = {topPlaneNormal, position};
                const glm::float3 bottomPlaneNormal = glm::cross(glm::normalize(farVector - up * halfHeight), right);
                frustum.GetBottom() = {bottomPlaneNormal, position};

				break;
			}
		case EProjectionType::Orthographic:
			{
				const float halfWidth = camera.mOrtoWidth * 0.5f;
				const float halfHeight = halfWidth / camera.mAspectRatio;

				frustum.GetRight() = {-right, position + right * halfWidth};
				frustum.GetBottom() = {right, position - right * halfWidth};
				frustum.GetTop() = {-up, position + up * halfHeight};
				frustum.GetBottom() = {up, position - up * halfHeight};

				break;
			}
		default:
			{
				TURBO_UNINPLEMENTED();
			}
		}

		return frustum;
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