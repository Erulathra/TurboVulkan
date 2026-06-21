#pragma once

namespace Turbo
{
	enum class EBufferFlags : uint8
	{
		None = 0,

		CreateMapped = 1 << 0,

		UniformBuffer = 1 << 1,
		StorageBuffer = 1 << 2,
		IndexBuffer = 1 << 3,

		IndirectBuffer = 1 << 4,

		TransferSrc = 1 << 5,

		AccelerationStructureStorage = 1 << 6,
		AccelerationStructureInput = 1 << 7,
	};
	DEFINE_ENUM_OPERATORS(EBufferFlags, uint8)

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
		StorageImage = 1 << 2,

		TransientAttachment = 1 << 3,
	};
	DEFINE_ENUM_OPERATORS(ETextureFlags, uint8)

	enum class EMSAASamples : uint8
	{
		One = 1,
		Two = 2,
		Four = 4,
		Eight = 8,
	};

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

	constexpr vk::SampleCountFlagBits ToVkSampleCountBits(EMSAASamples samples)
	{
		switch (samples) {
		case EMSAASamples::One:
			return vk::SampleCountFlagBits::e1;
		case EMSAASamples::Two:
			return vk::SampleCountFlagBits::e2;
		case EMSAASamples::Four:
			return vk::SampleCountFlagBits::e4;
		case EMSAASamples::Eight:
			return vk::SampleCountFlagBits::e8;
		default:
			TURBO_UNINPLEMENTED()
		}

		return vk::SampleCountFlagBits::e1;
	}
}
