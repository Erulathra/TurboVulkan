#pragma once

#include "Graphics/ForwardLightningHelpers.h"

namespace Turbo
{
	struct FLightComponent
	{
		glm::float3 mColor = glm::float3(1.f);
		float mIntensity = 1000.f;

		ELightType mType = ELightType::Point;
		float mRange = 5.f;
		float mInnerAngle = glm::radians(30.f);
		float mOuterAngle = glm::radians(60.f);
	};
}