#pragma once

#include <limits>

#include "CommonTypeDefs.h"
#include "Graphics/Resources.h"

namespace Turbo
{
	class FGPUDevice;
	struct FTexture;

	struct FRGPassHandle
	{
		[[nodiscard]] bool IsValid() const { return mIndex != std::numeric_limits<uint16>::max(); }

		uint32 mIndex = std::numeric_limits<uint16>::max();

		friend bool operator==(const FRGPassHandle& lhs, const FRGPassHandle& rhs)
		{
			return lhs.mIndex == rhs.mIndex;
		}

		friend bool operator!=(const FRGPassHandle& lhs, const FRGPassHandle& rhs)
		{
			return !(lhs == rhs);
		}
	};

	enum class ERGResourceType : uint8
	{
		Invalid,
		Texture,
		Buffer
	};

	struct FRGResourceHandle
	{
		static constexpr uint32 kTypeMask = 0xE0000000;
		static constexpr uint32 kExternalMax = 0x10000000;
		static constexpr uint32 kIndexMask = 0x0FFFFFFF;
		static constexpr uint32 kInvalidHandle = 0xFFFFFFFF;

		static_assert(kTypeMask | kExternalMax | kIndexMask == 0xFFFFFFFF);

		FRGResourceHandle() = default;

		FRGResourceHandle(ERGResourceType type, uint32 index, bool bExternal = false)
		{
			mHandle = static_cast<uint32>(type) << std::countr_zero(kTypeMask)
				| (bExternal ? 1u : 0u) << std::countr_zero(kExternalMax)
				| index << std::countr_zero(kIndexMask);
		}

		[[nodiscard]] bool IsValid() const { return mHandle != kInvalidHandle; }

		[[nodiscard]] ERGResourceType GetType() const
		{
			return static_cast<ERGResourceType>((mHandle & kTypeMask) >> std::countr_zero(kTypeMask));
		}

		[[nodiscard]] bool IsExternal() const { return (mHandle & kExternalMax) != 0; }
		[[nodiscard]] uint32 GetIndex() const { return (mHandle & kIndexMask) >> std::countr_zero(kIndexMask); }

		uint32 mHandle = kInvalidHandle;

		friend bool operator==(const FRGResourceHandle& lhs, const FRGResourceHandle& rhs)
		{
			return lhs.mHandle == rhs.mHandle;
		}

		friend bool operator!=(const FRGResourceHandle& lhs, const FRGResourceHandle& rhs)
		{
			return !(lhs == rhs);
		}
	};
}

template <>
struct std::hash<Turbo::FRGResourceHandle>
{
	size_t operator()(Turbo::FRGResourceHandle handle) const noexcept
	{
		return static_cast<uint64>(handle.mHandle) << 32;
	}
};

namespace Turbo
{
	enum class EPassType
	{
		Undefined,
		Graphics,
		Compute,
		Transfer
	};

	enum class ETextureLayout
	{
		Undefined,
		General,
		ReadOnly,

		ColorAttachment,
		DepthStencilAttachment,

		TransferSrc,
		TransferDst,

		PresentSrc
	};

	constexpr vk::ImageLayout ToVkImageLayout(ETextureLayout layout)
	{
		switch (layout)
		{
		case ETextureLayout::Undefined:
			return vk::ImageLayout::eUndefined;
		case ETextureLayout::General:
			return vk::ImageLayout::eGeneral;
		case ETextureLayout::ReadOnly:
			return vk::ImageLayout::eReadOnlyOptimal;
		case ETextureLayout::ColorAttachment:
			return vk::ImageLayout::eColorAttachmentOptimal;
		case ETextureLayout::DepthStencilAttachment:
			return vk::ImageLayout::eDepthStencilAttachmentOptimal;
		case ETextureLayout::TransferSrc:
			return vk::ImageLayout::eTransferSrcOptimal;
		case ETextureLayout::TransferDst:
			return vk::ImageLayout::eTransferDstOptimal;
		case ETextureLayout::PresentSrc:
			return vk::ImageLayout::ePresentSrcKHR;
		default: ;
		}

		TURBO_UNINPLEMENTED();
		return vk::ImageLayout::eUndefined;
	}

	constexpr ETextureLayout FromVkImageLayout(vk::ImageLayout layout)
	{
		switch (layout)
		{
		case vk::ImageLayout::eUndefined:
			return ETextureLayout::Undefined;
		case vk::ImageLayout::eGeneral:
			return ETextureLayout::General;
		case vk::ImageLayout::eReadOnlyOptimal:
			return ETextureLayout::ReadOnly;
		case vk::ImageLayout::eColorAttachmentOptimal:
			return ETextureLayout::ColorAttachment;
		case vk::ImageLayout::eDepthStencilAttachmentOptimal:
			return ETextureLayout::DepthStencilAttachment;
		case vk::ImageLayout::eTransferSrcOptimal:
			return ETextureLayout::TransferSrc;
		case vk::ImageLayout::eTransferDstOptimal:
			return ETextureLayout::TransferDst;
		case vk::ImageLayout::ePresentSrcKHR:
			return ETextureLayout::PresentSrc;
		default: ;
		}

		TURBO_UNINPLEMENTED();
		return ETextureLayout::Undefined;
	}

	enum class EResourceAccess
	{
		Read,
		ReadWrite,
	};

	struct FRGTextureInfo
	{
		uint16 mWidth = 1;
		uint16 mHeight = 1;

		vk::Format mFormat = vk::Format::eUndefined;

		FName mName = {};

		[[nodiscard]] bool IsValid() const;
	};

	struct FRGExternalTextureInfo
	{
		FRGTextureInfo mTextureInfo;

		THandle<FTexture> mTextureHandle;
		ETextureLayout mInitialLayout;
		ETextureLayout mFinalLayout;
	};

	struct FRGBufferInfo
	{
		FDeviceSize mSize = 0;
		vk::BufferUsageFlags2 mUsage = {};

		FName mName = {};
	};

	struct FRGImageMemoryBarrier
	{
		ETextureLayout mOldLayout = ETextureLayout::Undefined;
		ETextureLayout mNewLayout = ETextureLayout::Undefined;

		vk::PipelineStageFlags2 mSrcStageMask = vk::PipelineStageFlagBits2::eAllCommands;
		vk::PipelineStageFlags2 mDstStageMask = vk::PipelineStageFlagBits2::eAllCommands;

		vk::AccessFlags2 mSrcAccessMask = vk::AccessFlagBits2::eMemoryWrite;
		vk::AccessFlags2 mDstAccessMask = vk::AccessFlagBits2::eMemoryWrite | vk::AccessFlagBits2::eMemoryRead;

		FRGResourceHandle mTexture = {};

		vk::ImageMemoryBarrier2 ToVkImageBarrier(FGPUDevice& gpu, THandle<FTexture> textureHandle) const;
	};

	struct FRGResourceLifetime
	{
		uint16 mFirstPass = UINT16_MAX;
		uint16 mLastPass = 0;
	};
}
