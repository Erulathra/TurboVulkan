#pragma once
#include "CommonMacros.h"

namespace Turbo
{
	enum class EResourceUsageType : uint8
	{
		Immutable,
		Dynamic,
		Stream,

		Num
	};

	enum class ETextureType : uint8
	{
		Texture1D,
		Texture2D,
		Texture3D,

		Num
	};

	enum class ETextureFlags : uint8
	{
		Invalid = 0,

		Default = 1 << 0,
		RenderTarget = 1 << 1,
		Compute = 1 << 2,
	};
	DEFINE_ENUM_OPERATORS(ETextureFlags, uint8)
}
