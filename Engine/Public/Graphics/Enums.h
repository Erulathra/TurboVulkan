#pragma once

#include "CommonMacros.h"
#include "Graphics/GraphicsCore.h"

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

	enum class EQueueType : uint8
	{
		Graphics,
		Compute,
		CopyTransfer,

		Num
	};

	enum class EFilter : uint8
	{
		Nearest,
		Linear,
		Cubic,

		Num
	};

	namespace VkConvert
	{
		constexpr vk::ImageType ToVkImageType(ETextureType type)
		{
			switch (type) {
			case ETextureType::Texture1D:
				return vk::ImageType::e1D;
			case ETextureType::Texture2D:
				return vk::ImageType::e2D;
			case ETextureType::Texture3D:
				return vk::ImageType::e3D;
			default:
				TURBO_CHECK(false);
				break;
			}

			return vk::ImageType::e2D;
		}

		constexpr vk::ImageViewType ToVkImageViewType(ETextureType type)
		{
			switch (type) {
			case ETextureType::Texture1D:
				return vk::ImageViewType::e1D;
			case ETextureType::Texture2D:
				return vk::ImageViewType::e2D;
			case ETextureType::Texture3D:
				return vk::ImageViewType::e3D;
			default:
				TURBO_CHECK(false);
				break;
			}

			return vk::ImageViewType::e2D;

		}

		constexpr vk::Filter ToVkFilter(EFilter filter)
		{
			switch (filter)
			{
			case EFilter::Nearest:
				return vk::Filter::eNearest;
			case EFilter::Linear:
				return vk::Filter::eLinear;
			case EFilter::Cubic:
				return vk::Filter::eCubicIMG;
			default: ;
				TURBO_CHECK(false);
				break;
			}

			return vk::Filter::eLinear;
		}

	}
}
