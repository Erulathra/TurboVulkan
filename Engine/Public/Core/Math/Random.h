#pragma once

namespace Turbo
{
	namespace Random
	{
		void SetRandomSeed();
		void SetSeed(uint32 seed);

		uint32 RandomInt();
		float RandomFloat();

		int32 RandomRange(int32 min, int32 max);
		float RandomRange(float min = 0.f, float max = 1.f);

		glm::float3 RandomColor(float saturation = 1.f, float value = 1.f);
	} // Random
} // Turbo