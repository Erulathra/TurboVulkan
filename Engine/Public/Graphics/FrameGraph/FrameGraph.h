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
		static constexpr uint32 kTypeMask = 0xC0000000;
		static constexpr uint32 kVersionMask = 0x3FFF0000;
		static constexpr uint32 kIndexMask = 0x0000FFFF;
		static constexpr uint32 kInvalidHandle = 0xFFFFFFFF;

		FRGResourceHandle() = default;
		FRGResourceHandle(ERGResourceType type, uint32 index, uint32 version = 0)
		{
			mHandle = static_cast<uint32>(type) << std::countl_zero(kTypeMask)
				| version << std::countl_zero(kVersionMask)
				| index << std::countl_zero(kIndexMask);
		}

		[[nodiscard]] FRGResourceHandle IncrementVersion() const
		{
			FRGResourceHandle result = FRGResourceHandle(GetType(), GetIndex(), GetVersion() + 1);
			return result;
		}

		[[nodiscard]] bool IsValid() const { return mHandle != kInvalidHandle; }
		[[nodiscard]] ERGResourceType GetType() const { return static_cast<ERGResourceType>((mHandle & kTypeMask) >> std::countr_zero(kTypeMask)); }
		[[nodiscard]] uint32 GetVersion() const { return (mHandle & kVersionMask) >> std::countr_zero(kVersionMask); }
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

	struct FRGTextureReadInfo
	{
		FRGResourceHandle mHandle = {};
		vk::ImageLayout mTargetLayout = vk::ImageLayout::eUndefined;
	};

	enum class ECommandQueueType
	{
		Main,
		AsyncCompute,
		AsyncTransfer
	};

	DECLARE_DELEGATE(FRGSetupPassDelegate, FRenderGraphBuilder&, FRGPassInfo& );
	DECLARE_DELEGATE(FRGExecutePassDelegate, FGPUDevice&, FCommandBuffer&, FRenderResources&);

	struct FRGPassInfo
	{
		FRGResourceHandle CreateTexture(const FRGTextureInfo& textureInfo);
		FRGResourceHandle ReadTexture(FRGResourceHandle texture, vk::ImageLayout targetLayout);
		FRGResourceHandle WriteTexture(FRGResourceHandle texture);

		FRGResourceHandle AddAttachment(FRGResourceHandle texture, uint32 mAttachmentIndex);
		FRGResourceHandle SetDepthStencilAttachment(FRGResourceHandle texture);

		FRGResourceHandle CreateBuffer(const FRGBufferInfo& bufferInfo);
		FRGResourceHandle ReadBuffer(FRGResourceHandle handle);
		FRGResourceHandle WriteBuffer(FRGResourceHandle handle);

	public:
		std::vector<FRGResourceHandle> mTextureCreations;
		std::vector<FRGTextureReadInfo> mTextureReads;
		std::vector<FRGResourceHandle> mTextureWrites;

		std::vector<FRGResourceHandle> mBufferReads;
		std::vector<FRGResourceHandle> mBufferWrites;

		std::array<FRGResourceHandle, kMaxColorAttachments> mColorAttachments;
		FRGResourceHandle mDepthStencilAttachment = {};

		ECommandQueueType mCommandQueue = ECommandQueueType::Main;

		FRGExecutePassDelegate mExecutePass;

		FRenderGraphBuilder* mGraphBuilder = nullptr;
		FRGPassHandle mHandle = {};
		FName mName = {};
	};

	struct FRenderResources
	{
		std::vector<THandle<FTexture>> mTextures;
		std::vector<THandle<FBuffer>> mBuffers;
	};

	struct FRenderGraphBuilder
	{
		FRGPassInfo& AddPass(FName passName, const FRGSetupPassDelegate& setup, FRGExecutePassDelegate execute);

		void Compile();
		void Run();

		void CompileSynchronization();

	public:
		std::vector<FRGPassInfo> mRenderPasses;



		std::vector<FRGTextureInfo> mTextures;
		std::vector<FRGBufferInfo> mBuffers;
	};
} // Turbo

template<>
struct std::hash<Turbo::FRGResourceHandle>
{
	size_t operator()(Turbo::FRGResourceHandle handle) const noexcept
	{
		return static_cast<uint64>(handle.mHandle) << 32
			| static_cast<uint64>(handle.mOwner.mIndex);
	}
};
