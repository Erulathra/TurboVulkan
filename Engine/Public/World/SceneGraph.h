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
		glm::vec3 mPosition = {};
		glm::quat mRotation = {};
		glm::vec3 mScale = glm::vec3{1.f};
	};

	struct FWorldTransform
	{
		glm::mat4 mTransform = glm::mat4(1.f);

		operator glm::mat4() const
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