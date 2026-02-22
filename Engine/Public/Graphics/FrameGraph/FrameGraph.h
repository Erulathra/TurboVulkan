#pragma once

#include "Core/Delegate.h"
#include "Core/Allocators/StackAllocator.h"
#include "Graphics/GraphicsCore.h"
#include "Graphics/FrameGraph/FrameGraphHelpers.h"

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

		EPassType mPassType = EPassType::Undefined;

		FRGExecutePassDelegate mExecutePass;

		FRenderGraphBuilder* mGraphBuilder = nullptr;
		FRGPassHandle mHandle = {};
		FName mName = {};
	};

	struct FRenderResources
	{
		entt::dense_map<FRGResourceHandle, THandle<FTexture>> mTextures = {};
	};

	struct FRenderGraphBuilder
	{
		static constexpr uint32 kPerFrameStackSize = 4 * Constants::kKibi;

		DELETE_COPY(FRenderGraphBuilder)
		FRenderGraphBuilder() = default;

		[[nodiscard]] FRGResourceHandle AddTexture(const FRGTextureInfo& textureInfo);
		[[nodiscard]] FRGResourceHandle RegisterExternalTexture(THandle<FTexture> texture, ETextureLayout initLayout);
		[[nodiscard]] FRGResourceHandle RegisterExternalTexture(THandle<FTexture> texture, ETextureLayout initLayout, ETextureLayout finalLayout);

		FRGPassInfo& AddPass(FName passName, const FRGSetupPassDelegate& setup, FRGExecutePassDelegate&& execute);

		void Compile();
		void CompileResourceLifeTimes();
		void CompileTextureSynchronization();

		void Execute(FGPUDevice& gpu, FCommandBuffer& cmd);

		template <typename PODType>
		PODType& AllocatePOD()
		{
			return *mAllocator.Allocate<PODType>();
		}

		vk::Format GetTextureFormat(FRGResourceHandle resourceHandle) const;

	public:
		std::vector<FRGPassInfo> mRenderPasses;

		using FRGPassImageBarriers = std::vector<FRGImageMemoryBarrier>;
		std::vector<FRGPassImageBarriers> mPerPassImageBarriers;
		FRGPassImageBarriers mExternalResourcesBarriers;

		std::vector<FRGTextureInfo> mTextures;
		std::vector<FRGExternalTextureInfo> mExternalTextures;

		entt::dense_map<FRGResourceHandle, FRGResourceLifetime> mResourceLifetimes;

		FStackAllocator mAllocator = FStackAllocator(kPerFrameStackSize);
	};
} // Turbo
