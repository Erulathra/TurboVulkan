#pragma once

#include "glm/fwd.hpp"
#include "glm/vec3.hpp"
#include "glm/detail/type_quat.hpp"

namespace Turbo
{
	class FWorld;
}

namespace Turbo
{
	struct FTransform
	{
		glm::float3 mPosition = {};
		glm::quat mRotation = glm::quat(glm::float3(0.f));
		glm::float3 mScale = glm::float3{1.f};
	};

	struct FWorldTransform
	{
		glm::float4x4 mTransform = glm::float4x4(1.f);
	};

	struct FWorldTransformDirty {};

	struct FRelationship
	{
		uint32 mNumChildren = {};
		entt::entity mFirstChild = entt::null;
		entt::entity mPrevious = entt::null;
		entt::entity mNext = entt::null;
		entt::entity mParent = entt::null;
	};

	namespace TransformUtils
	{
		inline glm::float3 GetForward(const FTransform& transform)
		{
			return glm::normalize(transform.mRotation * EFloat3::Forward);
		}

		inline glm::float3 GetForward(const FWorldTransform& transform)
		{
			return glm::normalize(glm::float3(transform.mTransform[2]));
		}

		inline glm::float3 GetRight(const FTransform& transform)
		{
			return glm::normalize(transform.mRotation * EFloat3::Right);
		}

		inline glm::float3 GetRight(const FWorldTransform& transform)
		{
			return glm::normalize(glm::float3(transform.mTransform[0]));
		}

		inline glm::float3 GetUp(const FTransform& transform)
		{
			return glm::normalize(transform.mRotation * EFloat3::Up);
		}

		inline glm::float3 GetUp(const FWorldTransform& transform)
		{
			return glm::normalize(glm::float3(transform.mTransform[1]));
		}

		inline glm::float3 GetPosition(const FWorldTransform& transform)
		{
			return glm::float3(transform.mTransform[3]);
		}
	}

	struct FSceneGraph
	{
	public:
		static void InitSceneGraph(entt::registry& registry);

		static void UpdateWorldTransforms(entt::registry& registry);
		static void ClearDirtyFlags(entt::registry& registry);

		static void AddChild(entt::registry& registry, entt::entity parent, entt::entity child);
		static void Unparent(entt::registry& registry, entt::entity child);

		static void MarkDirty(entt::registry& registry, entt::entity entity);

		template<typename Func>
		static void EachChild(entt::registry& registry, entt::entity entity, Func func)
		{
			const FRelationship& componentRel = registry.get<FRelationship>(entity);
			entt::entity currentEntt = componentRel.mFirstChild;

			for (uint32 ChildId = 0; ChildId < componentRel.mNumChildren; ++ChildId)
			{
				std::invoke(func, currentEntt);

				const FRelationship& currentRel = registry.get<FRelationship>(currentEntt);
				currentEntt = currentRel.mNext;
			}
		}
	};
} // Turbo