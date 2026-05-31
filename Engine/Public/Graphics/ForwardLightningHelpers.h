#pragma once

namespace Turbo
{
	enum class ELightType : uint8
	{
		Point,
		Spot,
		Directional,

		MaxValue = Directional,
		Num
	};
	DEFINE_ENUM_OPERATORS(ELightType, uint8)

	inline const char* ToString(ELightType light)
	{
		switch (light)
		{
		case ELightType::Point:
			return "Point";
		case ELightType::Spot:
			return "Spot";
		case ELightType::Directional:
			return "Directional";
		default: ;
		}

		TURBO_UNINPLEMENTED()
	}

	// Update Light encoding code
	static_assert(static_cast<uint8>(ELightType::MaxValue) < (1 << 2));

	struct FLight
	{
		glm::float3 mColor = glm::float3(1.f);
		float mIntensity = 1000.f;
		glm::float3 mPosition = glm::float3(0.f);
		float mRange = 5.f;

		glm::float3 mDirection = EFloat3::Forward;
		uint32 mInnerOuterAngleAndType = 0;

		byte _PADDING[16];
	};

	namespace ForwardLightning
	{
		constexpr uint32 EncodeLightAnglesAndType(float InnerAngle, float OuterAngle, ELightType lightType)
		{
			constexpr uint32 MaxAngleValue = (1 << 15) - 1;

			const float InnerAngleNorm = glm::clamp<float>(InnerAngle, 0.0f, M_PI) / M_PI;
			const uint32 EncodedInnerAngle = static_cast<uint32>(InnerAngleNorm * MaxAngleValue) & MaxAngleValue;
			const float OuterAngleNorm = glm::clamp<float>(OuterAngle, 0.0f, M_PI) / M_PI;
			const uint32 EncodedOuterAngle = static_cast<uint32>(OuterAngleNorm * MaxAngleValue) & MaxAngleValue;

			return EncodedInnerAngle << 17 | EncodedOuterAngle << 2 | (static_cast<uint32>(lightType) & 3);
		}

		constexpr uint32 EncodeLightType(ELightType lightType)
		{
			constexpr uint32 MaxType = static_cast<uint32>(ELightType::MaxValue);
			return 0u | (static_cast<uint32>(lightType) & MaxType);
		}
	}
}
