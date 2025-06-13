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
	static T Min(const T& a, const T& b)
	{
		return glm::min(a, b);
	}

	template <typename T> requires (std::is_arithmetic_v<T>)
	static T Max(const T& a, const T& b)
	{
		return glm::max(a, b);
	}

	template <typename T> requires (std::is_arithmetic_v<T>)
	static T Clamp(const T& x, const T& min, const T& max)
	{
		return glm::clamp(x, min, max);
	}

	template <typename T> requires (std::is_arithmetic_v<T>)
	static T Abs(const T& a, const T& b)
	{
		return glm::abs(a, b);
	}

	template <typename T> requires (std::is_floating_point_v<T>)
	static bool NearlyEqual(const T& a, const T& b, const T& epsilon = static_cast<T>(TURBO_VERY_SMALL_NUMBER))
	{
		return glm::epsilonEqual(a, b, epsilon);
	}

	template <typename T> requires (std::is_floating_point_v<T>)
	static bool NearlyZero(const T& a, const T& epsilon = static_cast<T>(TURBO_VERY_SMALL_NUMBER))
	{
		return glm::epsilonEqual(a, static_cast<T>(0), epsilon);
	}

	template <typename RangeType, typename ValueType>
	static RangeType Lerp(const RangeType& a, const RangeType& b, const ValueType& x)
	{
		return glm::mix(a, b, x);
	}

	template <typename T> requires std::is_arithmetic_v<T>
	static T Sin(const T& x)
	{
		return glm::sin(x);
	}

	template <typename T> requires (std::is_arithmetic_v<T>)
	static T Cos(const T& x)
	{
		return glm::cos(x);
	}
};

} // Turbo
