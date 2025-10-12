#pragma once

#include "glm/glm.hpp"
#include "glm/gtc/epsilon.hpp"

// MATH
#define TURBO_SMALL_NUMBER 1e-5
#define TURBO_VERY_SMALL_NUMBER 1e-8

namespace Turbo
{
	namespace FMath
	{
		template <typename T>
			requires (std::is_floating_point_v<T>)
		static bool NearlyEqual(const T& lhs, const T& rhs, const T& epsilon = static_cast<T>(TURBO_VERY_SMALL_NUMBER))
		{
			return glm::epsilonEqual(lhs, rhs, epsilon);
		}

		template <typename T>
			requires (std::is_floating_point_v<T>)
		static bool NearlyZero(const T& lhs, const T& epsilon = static_cast<T>(TURBO_VERY_SMALL_NUMBER))
		{
			return glm::epsilonEqual(lhs, static_cast<T>(0), epsilon);
		}

		template <typename T>
		static T DivideAndRoundUp(T lhs, T rhs)
		{
			// return glm::ceil(lhs / rhs);
			return glm::ceil(lhs / rhs);
		}

		inline void ConvertTurboToVulkanCoordinates(glm::mat4& transform)
		{
			transform[1][1] = -transform[1][1];
		}

	};

	template <>
	inline glm::ivec3 FMath::DivideAndRoundUp(glm::ivec3 lhs, glm::ivec3 rhs)
	{
		return glm::ivec3(glm::ceil(glm::vec3(lhs) / glm::vec3(rhs)));
	}

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
