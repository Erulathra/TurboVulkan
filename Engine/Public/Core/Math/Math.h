#pragma once

#include "glm/glm.hpp"
#include "glm/gtc/epsilon.hpp"
#include "glm/gtx/quaternion.hpp"

// MATH
#define TURBO_SMALL_NUMBER 1e-5
#define TURBO_VERY_SMALL_NUMBER 1e-8

namespace glm
{
	typedef uint32						uint1;			//!< \brief unsigned integer vector with 1 component. (From GLM_GTX_compatibility extension)
	typedef vec<2, uint32, highp>		uint2;			//!< \brief unsigned integer vector with 2 components. (From GLM_GTX_compatibility extension)
	typedef vec<3, uint32, highp>		uint3;			//!< \brief unsigned integer vector with 3 components. (From GLM_GTX_compatibility extension)
	typedef vec<4, uint32, highp>		uint4;			//!< \brief unsigned integer vector with 4 components. (From GLM_GTX_compatibility extension)
}

namespace Turbo
{
	namespace FMath
	{
		template <typename T>
			requires (std::is_floating_point_v<T>)
		bool NearlyEqual(const T& lhs, const T& rhs, const T& epsilon = static_cast<T>(TURBO_VERY_SMALL_NUMBER))
		{
			return glm::epsilonEqual(lhs, rhs, epsilon);
		}

		template <typename T>
			requires (std::is_floating_point_v<T>)
		bool NearlyZero(const T& lhs, const T& epsilon = static_cast<T>(TURBO_VERY_SMALL_NUMBER))
		{
			return glm::epsilonEqual(lhs, static_cast<T>(0), epsilon);
		}

		template <typename T>
		T DivideAndRoundUp(T lhs, T rhs)
		{
			return glm::ceil(lhs / rhs);
		}

		inline void ConvertTurboToVulkanCoordinates(glm::float4x4& transform)
		{
			transform[1][1] = -transform[1][1];
		}

		inline glm::float4x4 CreateTransform(const glm::float3& position, const glm::quat& rotation, const glm::float3 scale)
		{
			const glm::float4x4 translationMat = glm::translate(glm::float4x4(1.f), position);
			const glm::float4x4 rotationMat = glm::toMat4(rotation);
			const glm::float4x4 scaleMat = glm::scale(glm::float4x4(1.f), scale);

			return translationMat * rotationMat * scaleMat;
		}

		template <typename T>
		T SafeNormal(T value)
		{
			if (glm::length(value) < TURBO_SMALL_NUMBER)
			{
				return T(0.f);
			}

			return glm::normalize(value);
		}

		template <typename T>
			requires std::is_floating_point_v<T>
		T NormalizeAngle(T radians)
		{
			constexpr T TwoPi = glm::two_pi<T>();

			radians = glm::mod<T>(radians, TwoPi);
			if (radians < 0.f)
			{
				radians += TwoPi;
			}

			return radians;
		}

	};

	template <>
	inline glm::int3 FMath::DivideAndRoundUp(glm::int3 lhs, glm::int3 rhs)
	{
		return glm::int3(glm::ceil(glm::float3(lhs) / glm::float3(rhs)));
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
