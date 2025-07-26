#pragma once

#include "MathCommon.h"

namespace Turbo
{
	namespace EVec2
	{
		const glm::vec2 One(1.f);
		const glm::vec2 Zero(0.f);

		const glm::vec2 Up(0.f, -1.f);
		const glm::vec2 Right(-1.f, 0.f);
	}

	namespace EVec3
	{
		const glm::vec3 One(1.f);
		const glm::vec3 Zero(0.f);

		const glm::vec3 Up(0.f, -1.f, 0.f);
		const glm::vec3 Right(1.f, 0.f, 0.f);
		const glm::vec3 Forward(0.f, 0.f, 1.f);
	}
}
