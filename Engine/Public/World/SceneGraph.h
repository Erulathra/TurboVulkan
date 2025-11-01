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

		operator glm::float4x4() const
		{
			return mTransform;
		}
	};

	struct FWorldTransformDirty { };

	struct FRelationship
	{
		uint32 mNumChildren = {};
		entt::entity mFirstChild = entt::null;
		entt::entity mPrevious = entt::null;
		entt::entity mNext = entt::null;
		entt::entity mParent = entt::null;
	};

} // Turbo