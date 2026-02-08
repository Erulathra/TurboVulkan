#include "Graphics/FrameGraph/FrameGraph.h"

#include "Graphics/CommandBuffer.h"
#include "Graphics/GPUDevice.h"
#include "Graphics/VulkanInitializers.h"

namespace Turbo
{
	FRGResourceHandle FRGPassInfo::CreateTexture(const FRGTextureInfo& textureInfo)
	{
		mGraphBuilder->mTextures.push_back(textureInfo);
		const FRGResourceHandle textureHandle(ERGResourceType::Texture, mGraphBuilder->mTextures.size() - 1);
		return WriteTexture(textureHandle);
	}

	FRGResourceHandle FRGPassInfo::ReadTexture(FRGResourceHandle texture)
	{
		mTextureReads.emplace_back(texture);
		return texture;
	}

	FRGResourceHandle FRGPassInfo::WriteTexture(FRGResourceHandle texture)
	{
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

	FRGPassInfo& FRenderGraphBuilder::AddPass(FName passName, const FRGSetupPassDelegate& setup, FRGExecutePassDelegate execute)
	{
		mRenderPasses.emplace_back();
		FRGPassInfo& passInfo = mRenderPasses.back();
		passInfo.mExecutePass = std::move(execute);
		passInfo.mGraphBuilder = this;
		passInfo.mHandle = { .mIndex = static_cast<uint16>(mRenderPasses.size() - 1) };
		passInfo.mName = passName;

		TURBO_CHECK(setup.IsBound())
		setup.Execute(*this, passInfo);

		return passInfo;
	}

	inline vk::AccessFlagBits2 FindSrcAccessMask(EResourceAccess srcAccess, EPassType dstPassType)
	{
		if (srcAccess == EResourceAccess::Read)
		{
			return vk::AccessFlagBits2::eShaderRead;
		}

		return dstPassType == EPassType::Compute
			? vk::AccessFlagBits2::eShaderWrite
			: vk::AccessFlagBits2::eColorAttachmentWrite;
	};

	inline vk::PipelineStageFlagBits2 FindSrcStageMask(EPassType srcPass)
	{
		return srcPass == EPassType::Compute
		   ? vk::PipelineStageFlagBits2::eComputeShader
		   : vk::PipelineStageFlagBits2::eColorAttachmentOutput;
	};

	inline vk::PipelineStageFlagBits2 FindDstStageMask(EPassType dstPass)
	{
		return dstPass == EPassType::Compute
		   ? vk::PipelineStageFlagBits2::eComputeShader
		   : vk::PipelineStageFlagBits2::eFragmentShader;
	};

	inline vk::ImageSubresourceRange FindSubresourceRange(vk::Format format)
	{
		vk::ImageAspectFlags aspectFlags = {};

		if (TextureFormat::HasDepth(format))
		{
			aspectFlags |= vk::ImageAspectFlagBits::eDepth;

			if (TextureFormat::HasStencil(format))
			{
				aspectFlags |= vk::ImageAspectFlagBits::eStencil;
			}
		}
		else
		{
			aspectFlags |= vk::ImageAspectFlagBits::eColor;
		}

		return VkInit::ImageSubresourceRange(aspectFlags);
	}

	void FRenderGraphBuilder::Compile()
	{
		TRACE_ZONE_SCOPED()

		// Compile lifetimes
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

		// Compile synchronization
		struct FResourceState
		{
			EPassType mLastUseType = EPassType::Graphics;
			EResourceAccess mAccess = EResourceAccess::Create;
			ETextureLayout mLayout = ETextureLayout::Undefined;
		};

		entt::dense_map<FRGResourceHandle, FResourceState> resourceData;

		mPerPassImageBarriers.clear();
		mPerPassImageBarriers.resize(mRenderPasses.size());

		for (uint32 passId = 0; passId < mRenderPasses.size(); ++passId)
		{
			const FRGPassInfo& pass = mRenderPasses[passId];

			std::vector<FRGResourceHandle> readOnlyResources;
			std::vector<FRGResourceHandle> writeOnlyResources;
			entt::dense_set<FRGResourceHandle> readWriteResources;

			std::vector<FRGImageMemoryBarrier>& passImageBarriers = mPerPassImageBarriers[passId];

			for (FRGResourceHandle write : pass.mTextureWrites)
			{
				for (FRGResourceHandle read : pass.mTextureReads)
				{
					if (read == write)
					{
						readWriteResources.insert(read);
						break;
					}

					writeOnlyResources.push_back(write);
				}
			}

			for (FRGResourceHandle read : pass.mTextureReads)
			{
				if (readWriteResources.contains(read) == false)
				{
					readOnlyResources.push_back(read);
				}
			}

			for (FRGResourceHandle read : readOnlyResources)
			{
				auto foundSrcData = resourceData.find(read);
				TURBO_CHECK_MSG(foundSrcData != resourceData.end(), "Pass tries to read non existent resource")

				FResourceState& srcData = foundSrcData->second;
				if (srcData.mAccess != EResourceAccess::Read
					|| srcData.mLayout != ETextureLayout::ReadOnly)
				{
					FRGImageMemoryBarrier& newBarrier = passImageBarriers.emplace_back();
					newBarrier.mOldLayout = srcData.mLayout;
					newBarrier.mNewLayout = ETextureLayout::ReadOnly;

					newBarrier.mSrcStageMask = FindSrcStageMask(srcData.mLastUseType);
					newBarrier.mDstStageMask = FindDstStageMask(pass.mPassType);

					newBarrier.mSrcAccessMask = FindSrcAccessMask(srcData.mAccess, pass.mPassType);

					newBarrier.mDstAccessMask = vk::AccessFlagBits2::eShaderRead;

					srcData.mLastUseType = pass.mPassType;
					srcData.mAccess = EResourceAccess::Read;
					srcData.mLayout = ETextureLayout::ReadOnly;
				}
			}

			for (FRGResourceHandle write : writeOnlyResources)
			{
				FRGImageMemoryBarrier& newBarrier = passImageBarriers.emplace_back();
				newBarrier.mDstStageMask = FindDstStageMask(pass.mPassType);

				if (auto srcData = resourceData.find(write);
					srcData != resourceData.end())
				{
					newBarrier.mOldLayout = srcData->second.mLayout;
					newBarrier.mSrcAccessMask = FindSrcAccessMask(srcData->second.mAccess, pass.mPassType);
					newBarrier.mSrcStageMask = FindSrcStageMask(srcData->second.mLastUseType);
				}
				else
				{
					// This pass creates new texture
					newBarrier.mOldLayout = ETextureLayout::Undefined;
					newBarrier.mSrcAccessMask = vk::AccessFlagBits2::eMemoryWrite;
					newBarrier.mSrcStageMask =  vk::PipelineStageFlagBits2::eNone;
				}

				if (pass.mPassType == EPassType::Compute)
				{
					newBarrier.mNewLayout = ETextureLayout::General;
					newBarrier.mDstAccessMask = vk::AccessFlagBits2::eShaderWrite;
				}
				else
				{
					if (pass.mDepthStencilAttachment == write)
					{
						newBarrier.mNewLayout = ETextureLayout::DepthStencilAttachment;
						newBarrier.mDstAccessMask = vk::AccessFlagBits2::eDepthStencilAttachmentWrite;
					}
					else
					{
						newBarrier.mNewLayout = ETextureLayout::ColorAttachment;
						newBarrier.mDstAccessMask = vk::AccessFlagBits2::eColorAttachmentWrite;
					}

				}

				FResourceState& currentData = resourceData[write];
				currentData.mLastUseType = pass.mPassType;
				currentData.mAccess = EResourceAccess::Write;
				currentData.mLayout = newBarrier.mNewLayout;
			}

			for (FRGResourceHandle readWrite : readWriteResources)
			{
				TURBO_CHECK(pass.mPassType != EPassType::Graphics)

				FRGImageMemoryBarrier& newBarrier = passImageBarriers.emplace_back();

				auto foundSrcData = resourceData.find(readWrite);
				TURBO_CHECK_MSG(foundSrcData != resourceData.end(), "Pass tries to read non existent resource")

				FResourceState& srcData = foundSrcData->second;
				newBarrier.mSrcStageMask = FindSrcStageMask(srcData.mLastUseType);
				newBarrier.mDstStageMask = FindDstStageMask(pass.mPassType);

				newBarrier.mSrcAccessMask = FindSrcAccessMask(srcData.mAccess, pass.mPassType);
				newBarrier.mDstAccessMask =
					pass.mPassType == EPassType::Compute
						? vk::AccessFlagBits2::eShaderWrite
						: vk::AccessFlagBits2::eColorAttachmentWrite;

				newBarrier.mOldLayout = srcData.mLayout;
				newBarrier.mNewLayout = ETextureLayout::General;

				srcData.mLastUseType = pass.mPassType;
				srcData.mAccess = EResourceAccess::ReadWrite;
				srcData.mLayout = ETextureLayout::General;
			}
		}
	}

	void FRenderGraphBuilder::Execute(FGPUDevice& gpu, FCommandBuffer& cmd)
	{
		TRACE_ZONE_SCOPED()

		FRenderResources renderResources = {};
		renderResources.mTextures.resize(mTextures.size());

		for (uint16 passId = 0; passId < mRenderPasses.size(); ++passId)
		{
			const FRGPassInfo& pass = mRenderPasses[passId];

			std::vector<FRGResourceHandle> mAllResources;
			mAllResources.reserve(pass.mTextureWrites.size() + pass.mTextureReads.size());
			mAllResources.append_range(pass.mTextureReads);
			mAllResources.append_range(pass.mTextureWrites);

			// Create uninitialized resources
			for (FRGResourceHandle resource : mAllResources)
			{
				if (renderResources.mTextures[resource.GetIndex()].IsValid() == false)
				{
					// TODO: create texture
				}
			}

			// Add barrier
			FRGPassImageBarriers& passImageBarriers = mPerPassImageBarriers[passId];

			std::vector<vk::ImageMemoryBarrier2> imageBarriers;
			imageBarriers.reserve(passImageBarriers.size());

			for (const FRGImageMemoryBarrier& rgBarrier : passImageBarriers)
			{
				THandle<FTexture> textureHandle = renderResources.mTextures[rgBarrier.mTexture.GetIndex()];
				const FTexture* texture = gpu.AccessTexture(textureHandle);
				const FTextureCold* textureCold = gpu.AccessTextureCold(textureHandle);
				TURBO_CHECK(texture)

				vk::ImageMemoryBarrier2& vkBarrier = imageBarriers.emplace_back();
				vkBarrier.srcStageMask = rgBarrier.mSrcStageMask;
				vkBarrier.srcAccessMask = rgBarrier.mSrcAccessMask;
				vkBarrier.dstStageMask = rgBarrier.mDstStageMask;
				vkBarrier.dstAccessMask = rgBarrier.mDstAccessMask;
				vkBarrier.oldLayout = ToVkImageLayout(rgBarrier.mOldLayout);
				vkBarrier.newLayout = ToVkImageLayout(rgBarrier.mNewLayout);
				vkBarrier.srcQueueFamilyIndex = vk::QueueFamilyIgnored;
				vkBarrier.dstQueueFamilyIndex = vk::QueueFamilyIgnored;
				vkBarrier.image = texture->mVkImage;
				vkBarrier.subresourceRange = FindSubresourceRange(textureCold->mFormat);
			}

			vk::DependencyInfo dependencyInfo = {};
			dependencyInfo.imageMemoryBarrierCount = imageBarriers.size();
			dependencyInfo.pImageMemoryBarriers = imageBarriers.data();

			cmd.PipelineBarrier(dependencyInfo);

			TURBO_CHECK(pass.mExecutePass.IsBound());
			pass.mExecutePass.Execute(gpu, cmd, renderResources);

			// Destroy unused textures
			for (FRGResourceHandle resource : mAllResources)
			{
				if (mResourceLifetimes[resource].mLastPass == passId)
				{
					// TODO: Destroy texture
				}
			}
		}
	}
} // Turbo