#pragma once
#include "Core/Delegate.h"
#include "Graphics/GraphicsCore.h"

namespace Turbo
{
	class FCommandBuffer;
	class FGPUDevice;
	struct FBuffer;
	struct FTexture;
	struct FRenderGraphBuilder;
	struct FRenderResources;
	struct FRGPassInfo;

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
		static constexpr uint32 kTypeMask = 0xF0000000;
		static constexpr uint32 kIndexMask = 0x0FFFFFFF;
		static constexpr uint32 kInvalidHandle = 0xFFFFFFFF;

		FRGResourceHandle() = default;
		FRGResourceHandle(ERGResourceType type, uint32 index)
		{
			mHandle = static_cast<uint32>(type) << std::countl_zero(kTypeMask)
				| index << std::countl_zero(kIndexMask);
		}

		[[nodiscard]] bool IsValid() const { return mHandle != kInvalidHandle; }
		[[nodiscard]] ERGResourceType GetType() const { return static_cast<ERGResourceType>((mHandle & kTypeMask) >> std::countr_zero(kTypeMask)); }
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

template<>
struct std::hash<Turbo::FRGResourceHandle>
{
	size_t operator()(Turbo::FRGResourceHandle handle) const noexcept
	{
		return static_cast<uint64>(handle.mHandle) << 32;
	}
};

namespace Turbo
{
	struct FRGTextureInfo
	{
		uint16 mWidth = 1;
		uint16 mHeight = 1;

		vk::Format mFormat = vk::Format::eUndefined;

		FName mName = {};
	};

	struct FRGBufferInfo
	{
		FDeviceSize mSize = 0;
		vk::BufferUsageFlags2 mUsage = {};

		FName mName = {};
	};

	enum class EPassType
	{
		Graphics,
		Compute,
	};

	enum class ETextureLayout
	{
		Undefined,
		General,
		ReadOnly,

		ColorAttachment,
		DepthStencilAttachment
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
		default: ;
		}

		TURBO_UNINPLEMENTED();
		return ETextureLayout::Undefined;
	}

	enum class EResourceAccess
	{
		Read,
		Write,
		ReadWrite,
		Create
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
	};

	struct FRGResourceLifetime
	{
		uint16 mFirstPass = UINT16_MAX;
		uint16 mLastPass = 0;
	};

	DECLARE_DELEGATE(FRGSetupPassDelegate, FRenderGraphBuilder&, FRGPassInfo& );
	DECLARE_DELEGATE(FRGExecutePassDelegate, FGPUDevice&, FCommandBuffer&, FRenderResources&);

	struct FRGPassInfo
	{
		FRGResourceHandle CreateTexture(const FRGTextureInfo& textureInfo);
		FRGResourceHandle ReadTexture(FRGResourceHandle texture);
		FRGResourceHandle WriteTexture(FRGResourceHandle texture);

		FRGResourceHandle AddAttachment(FRGResourceHandle texture, uint32 mAttachmentIndex);
		FRGResourceHandle SetDepthStencilAttachment(FRGResourceHandle texture);

	public:
		std::vector<FRGResourceHandle> mTextureReads;
		std::vector<FRGResourceHandle> mTextureWrites;

		std::array<FRGResourceHandle, kMaxColorAttachments> mColorAttachments;
		FRGResourceHandle mDepthStencilAttachment = {};

		EPassType mPassType = EPassType::Graphics;

		FRGExecutePassDelegate mExecutePass;

		FRenderGraphBuilder* mGraphBuilder = nullptr;
		FRGPassHandle mHandle = {};
		FName mName = {};
	};

	struct FRenderResources
	{
		std::vector<THandle<FTexture>> mTextures;
	};

	struct FRenderGraphBuilder
	{
		FRGPassInfo& AddPass(FName passName, const FRGSetupPassDelegate& setup, FRGExecutePassDelegate execute);

		void Compile();
		void Execute(FGPUDevice& gpu, FCommandBuffer& cmd);

	public:
		std::vector<FRGPassInfo> mRenderPasses;

		using FRGPassImageBarriers = std::vector<FRGImageMemoryBarrier>;
		std::vector<FRGPassImageBarriers> mPerPassImageBarriers;

		std::vector<FRGTextureInfo> mTextures;
		entt::dense_map<FRGResourceHandle, FRGResourceLifetime> mResourceLifetimes;
	};
} // Turbo
