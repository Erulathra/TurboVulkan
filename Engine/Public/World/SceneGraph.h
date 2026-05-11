#pragma once

#include "Core/Math/MathTypes.h"
#include "glm/fwd.hpp"
#include "glm/vec3.hpp"
#include "glm/detail/type_quat.hpp"

namespace Turbo
{
	class FWorld;
}

namespace Turbo
{
	struct FEntityLabel
	{
		FName mName;
	};

	struct FWorldTransform
	{
		glm::float4x4 mTransform = glm::float4x4(1.f);
	};

	struct FWorldRoot {};
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

		inline glm::float4x4 MatrixFromTransform(const FTransform& transform)
		{
			const glm::float4x4 translationMat = glm::translate(glm::float4x4(1.f), transform.mPosition);
			const glm::float4x4 rotationMat = glm::toMat4(transform.mRotation);
			const glm::float4x4 scaleMat = glm::scale(glm::float4x4(1.f), transform.mScale);

			return translationMat * rotationMat * scaleMat;
		}

		inline FTransform TransformFromMatrix(const glm::float4x4& matrix)
		{
			FTransform result;
			result.mPosition = glm::float3(matrix[3]);

			glm::float3x3 basis = glm::float3x3(matrix);
			result.mScale = glm::float3(glm::length(basis[0]), glm::length(basis[1]), glm::length(basis[2]));

			basis[0] /= result.mScale.x;
			basis[1] /= result.mScale.y;
			basis[2] /= result.mScale.z;

			result.mRotation = glm::quat(basis);

			return result;
		}

		inline bool IsFrontOf(const FWorldTransform& transform, const glm::float3& position)
		{
			const glm::float3 forward = GetForward(transform);
			const glm::float3 sourcePos = GetPosition(transform);

			return glm::dot(forward, position - sourcePos) > 0.f;
		}

		inline bool IsFrontOf(const FWorldTransform& source, const FWorldTransform& test)
		{
			return IsFrontOf(source, GetPosition(test));
		}
	}

	namespace SceneGraph
	{
		void InitSceneGraph(entt::registry& registry);

		void UpdateWorldTransforms(entt::registry& registry);
		void ClearDirtyFlags(entt::registry& registry);

		void AddChild(entt::registry& registry, entt::entity parent, entt::entity child);
		void Unparent(entt::registry& registry, entt::entity child);
		void MarkDirty(entt::registry& registry, entt::entity entity);

		template<typename Func>
		void EachChild(entt::registry& registry, entt::entity entity, Func func)
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

		glm::float4x4 GetParentWorldTransform(entt::registry& registry, entt::entity entity);
	};
} // Turbo