#include "Core/Math/Random.h"

#include <random>

#include "glm/gtx/color_space.hpp"

namespace Turbo
{
	uint32 gSeed = 0;

	void Random::SetRandomSeed()
	{
		std::random_device randomDevice;
		gSeed = randomDevice();
	}

	void Random::SetSeed(uint32 seed)
	{
		gSeed = seed;
	}

	uint32 Random::RandomInt()
	{
		std::mt19937 mt(gSeed);
		gSeed = mt();

		return gSeed;
	}

	float Random::RandomFloat()
	{
		return static_cast<float>(RandomInt()) / static_cast<float>(std::numeric_limits<uint32>::max());
	}

	int32 Random::RandomRange(int32 min, int32 max)
	{
		TURBO_CHECK(min < max);

		return (static_cast<int32>(RandomInt()) % (max - min)) + min;
	}

	float Random::RandomRange(float min, float max)
	{
		TURBO_CHECK(min < max);

		return RandomFloat() * (max - min) + min;
	}

	glm::float3 Random::RandomColor(float saturation, float value)
	{
		return glm::hsvColor(glm::float3(RandomFloat(), saturation, value));
	}
} // Turbo