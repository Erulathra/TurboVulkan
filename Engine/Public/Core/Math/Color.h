#pragma once

#define COLOR_FROM_HEX(hex) HexToLinearColor(hex)

namespace Turbo
{
	namespace ELinearColor
	{
		const glm::float4 kTransparent {0.f, 0.f, 0.f, 0.f};

		const glm::float4 kRed {1.f, 0.f, 0.f, 1.f};
		const glm::float4 kGreen {0.f, 1.f, 0.f, 1.f};
		const glm::float4 kBlue {0.f, 0.f, 1.f, 1.f};

		const glm::float4 kCyan {0.f, 1.f, 1.f, 1.f};
		const glm::float4 kMagenta {1.f, 0.f, 1.f, 1.f};
		const glm::float4 kYellow {1.f, 1.f, 0.f, 1.f};

		const glm::float4 kWhite {1.f, 1.f, 1.f, 1.f};
		const glm::float4 kBlack {0.f, 0.f, 0.f, 1.f};

		constexpr uint32 LinearColorToHex(glm::float4 color)
		{
			color = glm::saturate(color);
			return
				static_cast<uint32>(color.r * 0xFF) << 24
				| static_cast<uint32>(color.g * 0xFF) << 16
				| static_cast<uint32>(color.b * 0xFF) << 8
				| static_cast<uint32>(color.a * 0xFF) << 0;
		}

		constexpr glm::float4 HexToLinearColor(uint32 hex)
		{
			return glm::float4 {
				(hex >> 24) & 0xFF,
				(hex >> 16) & 0xFF,
				(hex >> 8) & 0xFF,
				(hex >> 0) & 0xFF
			};
		}
	};
}