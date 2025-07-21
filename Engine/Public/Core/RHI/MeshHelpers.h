#pragma once

#include "Core/Math/Vector.h"

namespace Turbo {

	/** Optimized to be used with std430 */
	struct FVertex
	{
		glm::vec3 Position;
		float UVX;
		glm::vec3 Normal;
		float UVY;

		FVertex()
			: Position(0.f)
			, UVX(0.f)
			, Normal(0.f)
			, UVY(0.f)
		{
		};
	};

	/** Optimized to be used with std430 */
	struct FVertexWithColor
	{
		glm::vec3 Position;
		float UVX;
		glm::vec3 Normal;
		float UVY;
		glm::vec4 Color;

		FVertexWithColor()
			: Position(0.f)
			, UVX(0.f)
			, Normal(0.f)
			, UVY(0.f)
			, Color(0.f)
		{
		};
	};
} // Turbo
