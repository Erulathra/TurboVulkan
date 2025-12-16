#pragma once
#include "Core/Math/FRotator.h"

namespace Turbo
{
	enum class EProjectionType : uint8
	{
		Perspective,
		Orthographic
	};

	struct FCamera final
	{
		float mOrtoWidth = 10.f;
		float mFov = glm::radians(60.f);
		float mAspectRatio = 16.f / 9.f;

		float mNearPlane = 0.1f;
		float mFarPlane = 1000.f;

		EProjectionType mProjectionType = EProjectionType::Perspective;

		/** auto bindings */
		static void on_construct(entt::registry &registry, const entt::entity entity);
		static void on_update(entt::registry &registry, const entt::entity entity);
	};

	struct FCameraCache
	{
		glm::float4x4 mProjectionMatrix = {1.f};
	};

	struct FFreeCamera
	{
		float mMinMovementSpeed = 1.f;
		float mMaxMovementSpeed = 1000.f;
		float mMovementSpeedFactor = 1.1f;
		float mMovementSpeed = 5.f;
		float mRotationSensitivity = glm::radians(0.16f);
		FRotator mRotator = FRotator(0.f);
	};

	struct FCameraDirty {};
	struct FMainViewport {};

	struct FViewData final
	{
		glm::float4x4 mProjectionMatrix = {1.f};
		glm::float4x4 mViewMatrix = {1.f};

		glm::float4x4 mWorldToProjection = {1.f};

		double mTime = 0.f;
		double mWorldTime = 0.f;
		double mDeltaTime = 0.f;
		int32 mFrameIndex = 0;
	};

	class FCameraUtils final
	{
	public:
		static void UpdateDirtyCameras(entt::registry& registry);
		static void UpdateFreeCameraPosition(entt::registry& registry, const glm::float3& movementInput, float deltaTime);
		static void UpdateFreeCameraRotation(entt::registry& registry, const glm::float2& deltaRotation);
		static void UpdateFreeCameraSpeed(entt::registry& registry, const int32 deltaSpeed);

	public:
		static void InitializeCamera(entt::registry& registry, entt::entity entity);
		static void InitializeFreeCamera(entt::registry& registry, entt::entity entity);

	public:
		FCameraUtils() = delete;
	};

} // Turbo