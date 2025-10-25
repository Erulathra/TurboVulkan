#pragma once

#include "glm/fwd.hpp"
#include "glm/vec3.hpp"
#include "glm/detail/type_quat.hpp"

namespace Turbo
{
	class FTransform final
	{
	private:
		glm::vec3 mPosition = {};
		glm::quat mRotation = {};
		glm::vec3 mScale = {};
	};

	class FSceneGraph
	{
	private:
		std::vector<glm::mat4> mWorld;
		std::vector<glm::mat4> mLocal;
		std::vector<uint32> mParent;

		// TPoolGrowable<FTransform> mTransforms;
		
	};
} // Turbo