#include "Core/Math/MathTypes.h"

namespace Turbo
{
	FPlane::FPlane(const glm::float3 normal, const float distance)
		: mNormal(normal)
		, mDistance(distance)
	{
		TURBO_CHECK_SLOW(glm::length(normal) - 1.f < TURBO_SMALL_NUMBER);
	}

	FPlane::FPlane(glm::float3 normal, glm::float3 point): mNormal(normal), mDistance(glm::dot(normal, point))
	{
		TURBO_CHECK_SLOW(glm::length(normal) - 1.f < TURBO_SMALL_NUMBER);
	}
} // Turbo