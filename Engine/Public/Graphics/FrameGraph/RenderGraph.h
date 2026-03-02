#pragma once

#include "Core/Delegate.h"
#include "Core/Allocators/StackAllocator.h"
#include "Graphics/GraphicsCore.h"
#include "Graphics/FrameGraph/RenderGraphHelpers.h"

DECLARE_LOG_CATEGORY(LogRenderGraph, Info, Display)

namespace Turbo
{
	struct FRenderResources;
	class FCommandBuffer;
	class FGPUDevice;
	struct FRGPassInfo;
	struct FRenderGraphBuilder;
	struct FTexture;

	DECLARE_DELEGATE(FRGSetupPassDelegate, FRGPassInfo& /*passInfo*/);
	DECLARE_DELEGATE(FRGExecutePassDelegate, FGPUDevice& /*gpu*/, FCommandBuffer& /*cmd*/, FRenderResources& /*resources*/);

	struct FRGPassInfo
	{
		FRGResourceHandle ReadTexture(FRGResourceHandle texture);
		FRGResourceHandle WriteTexture(FRGResourceHandle texture);

		FRGResourceHandle ReadBuffer(FRGResourceHandle buffer);
		FRGResourceHandle WriteBuffer(FRGResourceHandle buffer);

		FRGResourceHandle AddAttachment(FRGResourceHandle texture, uint32 mAttachmentIndex);
		FRGResourceHandle SetDepthStencilAttachment(FRGResourceHandle texture);

	public:
		std::vector<FRGResourceHandle> mTextureReads;
		std::vector<FRGResourceHandle> mTextureWrites;

		std::vector<FRGResourceHandle> mBufferReads;
		std::vector<FRGResourceHandle> mBufferWrites;

		std::array<FRGResourceHandle, kMaxColorAttachments> mColorAttachments;
		FRGResourceHandle mDepthStencilAttachment = {};

		EPassType mPassType = EPassType::Undefined;

		FRGExecutePassDelegate mExecutePass;

		FRenderGraphBuilder* mGraphBuilder = nullptr;
		FRGPassHandle mHandle = {};
		FName mName = {};
	};

	struct FRenderResources
	{
		entt::dense_map<FRGResourceHandle, THandle<FTexture>> mTextures = {};
		entt::dense_map<FRGResourceHandle, THandle<FBuffer>> mBuffers = {};
	};

	struct FRenderGraphBuilder
	{
		static constexpr uint32 kPerFrameStackSize = 4 * Constants::kKibi;

		DELETE_COPY(FRenderGraphBuilder)
		FRenderGraphBuilder() = default;

		[[nodiscard]] FRGResourceHandle AddTexture(const FRGTextureInfo& textureInfo);
		[[nodiscard]] FRGResourceHandle RegisterExternalTexture(THandle<FTexture> texture, ETextureLayout initLayout);
		[[nodiscard]] FRGResourceHandle RegisterExternalTexture(THandle<FTexture> texture, ETextureLayout initLayout, ETextureLayout finalLayout);

		[[nodiscard]] FRGResourceHandle AddBuffer(const FRGBufferInfo& bufferInfo);
		void QueueBufferUpload(const FRGBufferUpload& bufferUpload);

		FRGPassInfo& AddPass(FName passName, const FRGSetupPassDelegate& setup, FRGExecutePassDelegate&& execute);

		void Compile();
		void CompileTextureSynchronization();

		void Execute(FGPUDevice& gpu, FCommandBuffer& cmd);

		template <typename PODType>
		PODType* AllocatePOD()
		{
			return mAllocator.Allocate<PODType>();
		}

		vk::Format GetTextureFormat(FRGResourceHandle resourceHandle) const;

	public:
		std::vector<FRGPassInfo> mRenderPasses;

		using FRGPassTextureBarriers = std::vector<FRGTextureMemoryBarrier>;
		std::vector<FRGPassTextureBarriers> mPerPassTextureBarriers;
		FRGPassTextureBarriers mExternalTexturesBarriers;

		std::vector<FRGTextureInfo> mTextures;
		std::vector<FRGExternalTextureInfo> mExternalTextures;

		std::vector<FRGBufferInfo> mBuffers;
		std::vector<FRGBufferUpload> mQueuedBufferUploads;

		FArenaAllocator mAllocator = FArenaAllocator(kPerFrameStackSize);
	};
} // Turbo
