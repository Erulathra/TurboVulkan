#include "Graphics/FrameGraph/FrameGraph.h"

#include "Graphics/CommandBuffer.h"
#include "Graphics/GPUDevice.h"
#include "Graphics/FrameGraph/FrameGraphHelpers.h"

namespace Turbo
{
	FRGResourceHandle FRGPassInfo::CreateTexture(const FRGTextureInfo& textureInfo)
	{
		const FRGResourceHandle textureHandle = mGraphBuilder->AddTexture(textureInfo);
		return WriteTexture(textureHandle);
	}

	FRGResourceHandle FRGPassInfo::ReadTexture(FRGResourceHandle texture)
	{
		TURBO_CHECK(std::ranges::find(mTextureReads, texture) == mTextureReads.end())
		mTextureReads.emplace_back(texture);
		return texture;
	}

	FRGResourceHandle FRGPassInfo::WriteTexture(FRGResourceHandle texture)
	{
		TURBO_CHECK(std::ranges::find(mTextureWrites, texture) == mTextureWrites.end())
		mTextureWrites.emplace_back(texture);

		return texture;
	}


	FRGResourceHandle FRGPassInfo::AddAttachment(FRGResourceHandle texture, uint32 mAttachmentIndex)
	{
		TURBO_CHECK(mAttachmentIndex < kMaxColorAttachments);
		TURBO_CHECK(mColorAttachments[mAttachmentIndex].IsValid() == false)

		texture = WriteTexture(texture);
		mColorAttachments[mAttachmentIndex] = texture;

		return texture;
	}

	FRGResourceHandle FRGPassInfo::SetDepthStencilAttachment(FRGResourceHandle texture)
	{
		TURBO_CHECK(mDepthStencilAttachment.IsValid() == false)

		texture = WriteTexture(texture);
		mDepthStencilAttachment = texture;

		return texture;
	}

	FRGResourceHandle FRenderGraphBuilder::AddTexture(const FRGTextureInfo& textureInfo)
	{
		TURBO_CHECK(textureInfo.IsValid())
		mTextures.push_back(textureInfo);
		return {ERGResourceType::Texture, static_cast<uint32>(mTextures.size() - 1)};
	}

	FRGResourceHandle FRenderGraphBuilder::RegisterExternalTexture(THandle<FTexture> texture, ETextureLayout initLayout)
	{
		return RegisterExternalTexture(texture, initLayout, initLayout);
	}

	FRGResourceHandle FRenderGraphBuilder::RegisterExternalTexture(THandle<FTexture> texture, ETextureLayout initLayout, ETextureLayout finalLayout)
	{
		TURBO_CHECK(texture)

		auto findExternalTexture =
			[texture](const FRGExternalTextureInfo& externalTextureInfo)
			{
				return externalTextureInfo.mTextureHandle == texture;
			};

		TURBO_CHECK(std::ranges::find_if(mExternalTextures, findExternalTexture) == mExternalTextures.end());

		FGPUDevice& gpu = entt::locator<FGPUDevice>::value();
		const FTextureCold* textureCold = gpu.AccessTextureCold(texture);
		TURBO_CHECK(texture);

		FRGExternalTextureInfo externalTextureInfo = {
			.mTextureInfo = {
				.mWidth = textureCold->mWidth,
				.mHeight = textureCold->mHeight,
				.mFormat = textureCold->GetFormat(),
				.mName = textureCold->mName
			},
			.mTextureHandle = texture,
			.mInitialLayout = initLayout,
			.mFinalLayout = finalLayout
		};

		mExternalTextures.push_back(externalTextureInfo);
		const FRGResourceHandle resourceHandle(
			ERGResourceType::Texture,
			mExternalTextures.size() - 1,
			true
		);

		return resourceHandle;
	}

	FRGPassInfo& FRenderGraphBuilder::AddPass(FName passName, const FRGSetupPassDelegate& setup, FRGExecutePassDelegate&& execute)
	{
		mRenderPasses.emplace_back();
		FRGPassInfo& passInfo = mRenderPasses.back();
		passInfo.mExecutePass = std::move(execute);
		passInfo.mGraphBuilder = this;
		passInfo.mHandle = { .mIndex = static_cast<uint16>(mRenderPasses.size() - 1) };
		passInfo.mName = passName;

		TURBO_CHECK(setup.IsBound())
		TURBO_CHECK(passInfo.mExecutePass.IsBound())

		setup.Execute(passInfo);

		TURBO_CHECK(passInfo.mPassType != EPassType::Undefined)

		return passInfo;
	}

	inline vk::AccessFlags2 FindAccessMask(EResourceAccess passType)
	{
		switch (passType)
		{
		case EResourceAccess::Read:
			return vk::AccessFlagBits2::eMemoryRead;
		case EResourceAccess::ReadWrite:
			return vk::AccessFlagBits2::eMemoryWrite | vk::AccessFlagBits2::eMemoryRead;
		default:
			TURBO_UNINPLEMENTED()
		}

		std::unreachable();
	};

	inline vk::PipelineStageFlags2 FindStageMask(EPassType passType, vk::Format format)
	{
		switch (passType)
		{
		case EPassType::Undefined:
			return vk::PipelineStageFlagBits2::eAllCommands;
		case EPassType::Graphics:
			return TextureFormat::HasDepth(format)
				       ? vk::PipelineStageFlagBits2::eEarlyFragmentTests
				       : vk::PipelineStageFlagBits2::eColorAttachmentOutput;
		case EPassType::Compute:
			return vk::PipelineStageFlagBits2::eComputeShader;
		case EPassType::Transfer:
			return vk::PipelineStageFlagBits2::eTransfer;
		default:
			TURBO_UNINPLEMENTED()
		}

		std::unreachable();
	};

	void FRenderGraphBuilder::CompileTextureSynchronization()
	{
		struct FResourceState
		{
			EPassType mLastUseType = EPassType::Graphics;
			EResourceAccess mAccess = EResourceAccess::ReadWrite;
			ETextureLayout mLayout = ETextureLayout::Undefined;
			vk::Format mFormat = vk::Format::eUndefined;
		};

		entt::dense_map<FRGResourceHandle, FResourceState> resourceData;

		// register external resources
		for (uint32 externalTextureId = 0; externalTextureId < mExternalTextures.size(); ++externalTextureId)
		{
			FRGResourceHandle resourceHandle(ERGResourceType::Texture, externalTextureId, true);
			resourceData[resourceHandle] = {
				.mLastUseType = EPassType::Undefined,
				.mAccess = EResourceAccess::ReadWrite,
				.mLayout = mExternalTextures[externalTextureId].mInitialLayout,
				.mFormat = mExternalTextures[externalTextureId].mTextureInfo.mFormat
			};
		}

		mPerPassImageBarriers.clear();
		mPerPassImageBarriers.resize(mRenderPasses.size());

		for (uint32 passId = 0; passId < mRenderPasses.size(); ++passId)
		{
			const FRGPassInfo& pass = mRenderPasses[passId];

			std::vector<FRGResourceHandle> readWriteResources = pass.mTextureWrites;
			std::vector<FRGResourceHandle> readOnlyResources;

			std::vector<FRGImageMemoryBarrier>& passImageBarriers = mPerPassImageBarriers[passId];

			for (FRGResourceHandle read : pass.mTextureReads)
			{
				if (std::ranges::find(readWriteResources, read) == readWriteResources.end())
				{
					readOnlyResources.push_back(read);
				}
			}

			for (FRGResourceHandle read : readOnlyResources)
			{
				auto foundSrcData = resourceData.find(read);
				TURBO_CHECK_MSG(foundSrcData != resourceData.end(), "Pass tries to read non existent resource")

				if (FResourceState& srcData = foundSrcData->second;
					srcData.mAccess != EResourceAccess::Read || srcData.mLayout != ETextureLayout::ReadOnly)
				{
					FRGImageMemoryBarrier& newBarrier = passImageBarriers.emplace_back();
					newBarrier.mTexture = read;
					newBarrier.mOldLayout = srcData.mLayout;
					newBarrier.mNewLayout =
						pass.mPassType == EPassType::Transfer
							? ETextureLayout::TransferSrc
							: ETextureLayout::ReadOnly;

					newBarrier.mSrcStageMask = FindStageMask(srcData.mLastUseType, srcData.mFormat);
					newBarrier.mDstStageMask = FindStageMask(pass.mPassType, srcData.mFormat);

					newBarrier.mSrcAccessMask = FindAccessMask(srcData.mAccess);
					newBarrier.mDstAccessMask =
						pass.mPassType == EPassType::Transfer
							? vk::AccessFlagBits2::eTransferRead
							: vk::AccessFlagBits2::eShaderRead;

					srcData.mLastUseType = pass.mPassType;
					srcData.mAccess = EResourceAccess::Read;
					srcData.mLayout = ETextureLayout::ReadOnly;
				}
			}

			for (FRGResourceHandle write : readWriteResources)
			{
				FRGImageMemoryBarrier& newBarrier = passImageBarriers.emplace_back();
				newBarrier.mTexture = write;
				newBarrier.mDstAccessMask = FindAccessMask(EResourceAccess::ReadWrite);

				if (auto srcDataIt = resourceData.find(write);
					srcDataIt != resourceData.end())
				{
					newBarrier.mOldLayout = srcDataIt->second.mLayout;
					newBarrier.mSrcAccessMask = FindAccessMask(srcDataIt->second.mAccess);
					newBarrier.mSrcStageMask = FindStageMask(srcDataIt->second.mLastUseType, srcDataIt->second.mFormat);
				}
				else
				{
					// This pass creates new texture
					newBarrier.mOldLayout = ETextureLayout::Undefined;
					newBarrier.mSrcAccessMask = vk::AccessFlagBits2::eNone;
					newBarrier.mSrcStageMask = vk::PipelineStageFlagBits2::eNone;

					resourceData[write] = {
						.mFormat = GetTextureFormat(write)
					};
				}

				switch (pass.mPassType)
				{
				case EPassType::Graphics:
					newBarrier.mNewLayout = pass.mDepthStencilAttachment == write
						                        ? ETextureLayout::DepthStencilAttachment
						                        : ETextureLayout::ColorAttachment;
					break;
				case EPassType::Compute:
					newBarrier.mNewLayout = ETextureLayout::General;
					break;
				case EPassType::Transfer:
					newBarrier.mNewLayout = ETextureLayout::TransferDst;
					break;
				default:
					TURBO_UNINPLEMENTED()
				}

				FResourceState& currentData = resourceData.at(write);
				newBarrier.mDstStageMask = FindStageMask(pass.mPassType, currentData.mFormat);

				currentData.mLastUseType = pass.mPassType;
				currentData.mAccess = EResourceAccess::ReadWrite;
				currentData.mLayout = newBarrier.mNewLayout;
			}
		}

		// Add final exterior resources barriers
		for (uint32 externalTextureId = 0; externalTextureId < mExternalTextures.size(); ++externalTextureId)
		{
			FRGResourceHandle resourceHandle(ERGResourceType::Texture, externalTextureId, true);
			if (auto foundSrcData = resourceData.find(resourceHandle);
				foundSrcData != resourceData.end())
			{
				FResourceState& srcData = foundSrcData->second;

				FRGImageMemoryBarrier& newBarrier = mExternalResourcesBarriers.emplace_back();
				newBarrier.mSrcStageMask = FindStageMask(srcData.mLastUseType, srcData.mFormat);
				newBarrier.mDstStageMask = vk::PipelineStageFlagBits2::eAllCommands;

				newBarrier.mSrcAccessMask = FindAccessMask(srcData.mAccess);
				newBarrier.mDstAccessMask = FindAccessMask(EResourceAccess::ReadWrite);

				newBarrier.mOldLayout = srcData.mLayout;
				newBarrier.mNewLayout = mExternalTextures[externalTextureId].mFinalLayout;

				newBarrier.mTexture = resourceHandle;
			}
		}
	}

	void FRenderGraphBuilder::CompileResourceLifeTimes()
	{
		for (uint16 passId = 0; passId < mRenderPasses.size(); ++passId)
		{
			const FRGPassInfo& pass = mRenderPasses[passId];
			for (FRGResourceHandle write : pass.mTextureWrites)
			{
				FRGResourceLifetime& lifetime = mResourceLifetimes[write];
				lifetime.mFirstPass = glm::min(lifetime.mFirstPass, passId);
				lifetime.mLastPass = glm::max(lifetime.mFirstPass, passId);
			}

			for (FRGResourceHandle read : pass.mTextureReads)
			{
				FRGResourceLifetime& lifetime = mResourceLifetimes[read];
				lifetime.mFirstPass = glm::min(lifetime.mFirstPass, passId);
				lifetime.mLastPass = glm::max(lifetime.mFirstPass, passId);
			}
		}
	}

	void FRenderGraphBuilder::Compile()
	{
		TRACE_ZONE_SCOPED()

		// Compile lifetimes
		CompileResourceLifeTimes();
		CompileTextureSynchronization();
	}

	void FRenderGraphBuilder::Execute(FGPUDevice& gpu, FCommandBuffer& cmd)
	{
		TRACE_ZONE_SCOPED()
		TURBO_LOG(LogRenderGraph, Display, "Executing render graph");

		FRenderResources renderResources = {};
		renderResources.mTextures.reserve(mTextures.size() + mExternalTextures.size());

		// Allocate resources
		for (uint32 textureId = 0; textureId < mTextures.size(); ++textureId)
		{
			const FRGTextureInfo& textureInfo = mTextures[textureId];
			TURBO_LOG(LogRenderGraph, Display, "Allocating texture: {}", textureInfo.mName);

			FTextureBuilder builder = {
				.mWidth = textureInfo.mWidth,
				.mHeight = textureInfo.mHeight,
				.mFlags = ETextureFlags::RenderTarget | ETextureFlags::StorageImage,
				.mFormat = textureInfo.mFormat,
				.mType = ETextureType::Texture2D,
				.mName = textureInfo.mName
			};

			const FRGResourceHandle handle(ERGResourceType::Texture, textureId, false);
			const THandle<FTexture> texture = gpu.CreateTexture(builder);
			TURBO_CHECK(texture)

			renderResources.mTextures.emplace(handle, texture);
		}

		for (uint32 externalTextureId = 0; externalTextureId < mExternalTextures.size(); ++externalTextureId)
		{
			const THandle<FTexture> texture = mExternalTextures[externalTextureId].mTextureHandle;
			TURBO_CHECK(texture)

			const FRGResourceHandle handle(ERGResourceType::Texture, externalTextureId, true);
			renderResources.mTextures.emplace(handle, texture);

			TURBO_LOG(LogRenderGraph, Display, "Registering external texture: {}", mExternalTextures[externalTextureId].mTextureInfo.mName);
		}

		for (uint32 passId = 0; passId < mRenderPasses.size(); ++passId)
		{
			const FRGPassInfo& pass = mRenderPasses[passId];
			TURBO_LOG(LogRenderGraph, Display, "Begin render pass: {}", pass.mName);

			// Add barrier
			FRGPassImageBarriers& passImageBarriers = mPerPassImageBarriers[passId];

			std::vector<vk::ImageMemoryBarrier2> imageBarriers;
			imageBarriers.reserve(passImageBarriers.size());

			for (const FRGImageMemoryBarrier& rgBarrier : passImageBarriers)
			{
				THandle<FTexture> textureHandle = renderResources.mTextures.at(rgBarrier.mTexture);
				imageBarriers.push_back(rgBarrier.ToVkImageBarrier(gpu, textureHandle));

				TURBO_LOG(
					LogRenderGraph, Display, "[Image Barrier] Texture: {}; {} -> {}",
					gpu.AccessTextureCold(textureHandle)->mName,
					magic_enum::enum_name(rgBarrier.mOldLayout),
					magic_enum::enum_name(rgBarrier.mNewLayout)
				);
			}

			vk::DependencyInfo dependencyInfo = {};
			dependencyInfo.imageMemoryBarrierCount = imageBarriers.size();
			dependencyInfo.pImageMemoryBarriers = imageBarriers.data();

			cmd.PipelineBarrier(dependencyInfo);

			if (pass.mPassType == EPassType::Graphics)
			{
				FRenderingAttachments renderingAttachments;

				// Bind color attachments
				for (uint32 attachmentId = 0; attachmentId < kMaxColorAttachments; ++attachmentId)
				{
					if (pass.mColorAttachments[attachmentId].IsValid())
					{
						const FRGResourceHandle attachmentResource = pass.mColorAttachments[attachmentId];

						const FAttachment attachmentInfo = {
							.mTexture = renderResources.mTextures.at(attachmentResource),
							.mLoadOp = std::ranges::contains(pass.mTextureReads, attachmentResource) ? ELoadOp::Load : ELoadOp::Clear,
							.mStoreOp = EStoreOp::Store,
							.mClearColor = EClearColor::TransparentBlack
						};
						renderingAttachments.AddColorAttachment(attachmentInfo);

						TURBO_LOG(
							LogRenderGraph, Display, "[GraphicsPass] Bind {} as color attachment {}",
							gpu.AccessTextureCold(renderResources.mTextures.at(attachmentResource))->mName,
							attachmentId
						);
					}
				}

				// Bind depth stencil attachment (if valid)
				if (pass.mDepthStencilAttachment.IsValid())
				{
					const FAttachment attachmentInfo = {
						.mTexture = renderResources.mTextures.at(pass.mDepthStencilAttachment),
						.mLoadOp = std::ranges::contains(pass.mTextureReads, pass.mDepthStencilAttachment) ? ELoadOp::Load : ELoadOp::Clear,
						.mStoreOp = EStoreOp::Store,
						.mClearColor = EClearColor::Zero
					};
					renderingAttachments.SetDepthAttachment(attachmentInfo);

					TURBO_LOG(
						LogRenderGraph, Display, "[GraphicsPass] Bind {} as depth attachment",
						gpu.AccessTextureCold(renderResources.mTextures[pass.mDepthStencilAttachment])->mName
					);
				}

				const FRGResourceHandle mainTextureHandle =
					pass.mColorAttachments[0].IsValid()
						? pass.mColorAttachments[0]
						: pass.mDepthStencilAttachment;

				TURBO_CHECK(mainTextureHandle.IsValid())

				const FRGTextureInfo& textureInfo =
					mainTextureHandle.IsExternal()
						? mExternalTextures[mainTextureHandle.GetIndex()].mTextureInfo
						: mTextures[mainTextureHandle.GetIndex()];
				const glm::ivec2 outputSize = glm::ivec2(textureInfo.mWidth, textureInfo.mHeight);

				cmd.BeginRendering(renderingAttachments);
				cmd.SetViewport(FViewport::FromSize(outputSize));
				cmd.SetScissor(FRect2DInt::FromSize(outputSize));

				TURBO_LOG(LogRenderGraph, Display, "[GraphicsPass] Begin rendering. Viewport: {} Scissors: {}", outputSize, outputSize);
			}

			TURBO_CHECK(pass.mExecutePass.IsBound());

			TURBO_LOG(LogRenderGraph, Display, "Execute: {}", pass.mName);
			pass.mExecutePass.Execute(gpu, cmd, renderResources);

			if (pass.mPassType == EPassType::Graphics)
			{
				TURBO_LOG(LogRenderGraph, Display, "[GraphicsPass] End rendering.");
				cmd.EndRendering();
			}

		}

		// Destroy resources
		for (uint32 textureId = 0; textureId < mTextures.size(); ++textureId)
		{
			const FRGResourceHandle handle(ERGResourceType::Texture, textureId, false);
			auto foundIt = renderResources.mTextures.find(handle);
			TURBO_CHECK(foundIt != renderResources.mTextures.end())

			TURBO_LOG(LogRenderGraph, Display, "Destroying texture: {}", gpu.AccessTextureCold(foundIt->second)->mName);
			gpu.DestroyTexture(foundIt->second);
		}

		// Transition external resources to their target layouts
		TURBO_LOG(LogRenderGraph, Display, "Final external resources barrier.");
		std::vector<vk::ImageMemoryBarrier2> imageBarriers;
		imageBarriers.reserve(mExternalResourcesBarriers.size());

		for (const FRGImageMemoryBarrier& rgBarrier : mExternalResourcesBarriers)
		{
			const THandle<FTexture> textureHandle = renderResources.mTextures.at(rgBarrier.mTexture);
			imageBarriers.push_back(rgBarrier.ToVkImageBarrier(gpu, textureHandle));

			TURBO_LOG(
				LogRenderGraph, Display, "[Image Barrier] Texture: {}; {} -> {}",
				gpu.AccessTextureCold(textureHandle)->mName,
				magic_enum::enum_name(rgBarrier.mOldLayout),
				magic_enum::enum_name(rgBarrier.mNewLayout)
			);
		}

		vk::DependencyInfo dependencyInfo = {};
		dependencyInfo.imageMemoryBarrierCount = imageBarriers.size();
		dependencyInfo.pImageMemoryBarriers = imageBarriers.data();

		cmd.PipelineBarrier(dependencyInfo);
	}

	vk::Format FRenderGraphBuilder::GetTextureFormat(FRGResourceHandle resourceHandle) const
	{
		TURBO_CHECK(resourceHandle.GetType() == ERGResourceType::Texture)

		return resourceHandle.IsExternal()
			       ? mExternalTextures[resourceHandle.GetIndex()].mTextureInfo.mFormat
			       : mTextures[resourceHandle.GetIndex()].mFormat;
	}
} // Turbo