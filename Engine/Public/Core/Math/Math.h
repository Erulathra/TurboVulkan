#pragma once

#include "glm/glm.hpp"
#include "glm/gtc/epsilon.hpp"

// MATH
#define TURBO_SMALL_NUMBER 1e-5
#define TURBO_VERY_SMALL_NUMBER 1e-8

namespace Turbo
{
	class FMath
	{
	public:
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
	};
} // Turbo

template <int32 L, typename T>
struct fmt::formatter<glm::vec<L, T>> : fmt::formatter<std::string>
{
	auto format(const glm::vec<L, T>& vector, format_context& ctx) const
	{
		std::stringstream resultStream;
		resultStream << '(';
		for (int componentIndex = 0; componentIndex < vector.length() - 1; ++componentIndex)
		{
			resultStream << vector[componentIndex] << ", ";
		}
		resultStream << vector[vector.length() - 1] << ")";

		return formatter<std::string>::format(resultStream.str(), ctx);
	}
};
