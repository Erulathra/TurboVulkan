#pragma once

#include "glm/glm.hpp"
#include "glm/gtc/epsilon.hpp"

// MATH
#define TURBO_SMALL_NUMBER 1e-5
#define TURBO_VERY_SMALL_NUMBER 1e-8

namespace Turbo {

class FMath {
public:
	template <typename T> requires (std::is_arithmetic_v<T>)
	static T Min(const T& A, const T& B)
	{
		return glm::min(A, B);
	}

	template <typename T> requires (std::is_arithmetic_v<T>)
	static T Max(const T& A, const T& B)
	{
		return glm::max(A, B);
	}

	template <typename T> requires (std::is_arithmetic_v<T>)
	static T Clamp(const T& X, const T& Min, const T& Max)
	{
		return glm::clamp(X, Min, Max);
	}

	template <typename T> requires (std::is_arithmetic_v<T>)
	static T Abs(const T& A, const T& B)
	{
		return glm::abs(A, B);
	}

	template <typename T> requires (std::is_floating_point_v<T>)
	static bool NearlyEqual(const T& A, const T& B, const T& Epsilon = static_cast<T>(TURBO_VERY_SMALL_NUMBER))
	{
		return glm::epsilonEqual(A, B, Epsilon);
	}

	template <typename T> requires (std::is_floating_point_v<T>)
	static bool NearlyZero(const T& A, const T& Epsilon = static_cast<T>(TURBO_VERY_SMALL_NUMBER))
	{
		return glm::epsilonEqual(A, static_cast<T>(0), Epsilon);
	}
};

} // Turbo
