#pragma once

namespace Turbo
{
	namespace EFloat2
	{
		constexpr glm::float2 One(1.f);
		constexpr glm::float2 Zero(0.f);

		constexpr glm::float2 Up(0.f, 1.f);
		constexpr glm::float2 Right(1.f, 0.f);
	}

	namespace EFloat3
	{
		constexpr glm::float3 One(1.f);
		constexpr glm::float3 Zero(0.f);

		constexpr glm::float3 Up(0.f, 1.f, 0.f);
		constexpr glm::float3 Right(1.f, 0.f, 0.f);
		constexpr glm::float3 Forward(0.f, 0.f, 1.f);
	}
}
