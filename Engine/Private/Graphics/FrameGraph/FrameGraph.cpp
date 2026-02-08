#include "Graphics/FrameGraph/FrameGraph.h"

#include "Graphics/CommandBuffer.h"
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

	inline vk::ImageSubresourceRange FindSubresourceRange(const FRGTextureInfo& textureInfo)
	{
		vk::ImageAspectFlags aspectFlags = {};

		if (TextureFormat::HasDepth(textureInfo.mFormat))
		{
			aspectFlags |= vk::ImageAspectFlagBits::eDepth;

			if (TextureFormat::HasStencil(textureInfo.mFormat))
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

			std::vector<vk::ImageMemoryBarrier2>& passImageBarriers = mPerPassImageBarriers[passId];

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
					vk::ImageMemoryBarrier2& newBarrier = passImageBarriers.emplace_back();
					newBarrier.srcQueueFamilyIndex = vk::QueueFamilyIgnored;
					newBarrier.dstQueueFamilyIndex = vk::QueueFamilyIgnored;
					newBarrier.subresourceRange = FindSubresourceRange(mTextures[read.GetIndex()]);

					newBarrier.oldLayout = ToVkImageLayout(srcData.mLayout);
					newBarrier.newLayout = ToVkImageLayout(ETextureLayout::ReadOnly);

					newBarrier.srcStageMask = FindSrcStageMask(srcData.mLastUseType);
					newBarrier.dstStageMask = FindDstStageMask(pass.mPassType);

					newBarrier.srcAccessMask = FindSrcAccessMask(srcData.mAccess, pass.mPassType);

					newBarrier.dstAccessMask = vk::AccessFlagBits2::eShaderRead;

					srcData.mLastUseType = pass.mPassType;
					srcData.mAccess = EResourceAccess::Read;
					srcData.mLayout = ETextureLayout::ReadOnly;
				}
			}

			for (FRGResourceHandle write : writeOnlyResources)
			{
				vk::ImageMemoryBarrier2& newBarrier = passImageBarriers.emplace_back();
				newBarrier.srcQueueFamilyIndex = vk::QueueFamilyIgnored;
				newBarrier.dstQueueFamilyIndex = vk::QueueFamilyIgnored;
				newBarrier.subresourceRange = FindSubresourceRange(mTextures[write.GetIndex()]);

				newBarrier.dstStageMask = FindDstStageMask(pass.mPassType);

				if (auto srcData = resourceData.find(write);
					srcData != resourceData.end())
				{
					newBarrier.oldLayout = ToVkImageLayout(srcData->second.mLayout);
					newBarrier.srcAccessMask = FindSrcAccessMask(srcData->second.mAccess, pass.mPassType);
					newBarrier.srcStageMask = FindSrcStageMask(srcData->second.mLastUseType);
				}
				else
				{
					// This pass creates new texture
					newBarrier.oldLayout = vk::ImageLayout::eUndefined;
					newBarrier.srcAccessMask = vk::AccessFlagBits2::eMemoryWrite;
					newBarrier.srcStageMask =  vk::PipelineStageFlagBits2::eNone;
				}

				if (pass.mPassType == EPassType::Compute)
				{
					newBarrier.newLayout = vk::ImageLayout::eGeneral;
					newBarrier.dstAccessMask = vk::AccessFlagBits2::eShaderWrite;
				}
				else
				{
					if (pass.mDepthStencilAttachment == write)
					{
						newBarrier.newLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
						newBarrier.dstAccessMask = vk::AccessFlagBits2::eDepthStencilAttachmentWrite;
					}
					else
					{
						newBarrier.newLayout = vk::ImageLayout::eColorAttachmentOptimal;
						newBarrier.dstAccessMask = vk::AccessFlagBits2::eColorAttachmentWrite;
					}

				}

				FResourceState& currentData = resourceData[write];
				currentData.mLastUseType = pass.mPassType;
				currentData.mAccess = EResourceAccess::Write;
				currentData.mLayout = FromVkImageLayout(newBarrier.newLayout);
			}

			for (FRGResourceHandle readWrite : readWriteResources)
			{
				TURBO_CHECK(pass.mPassType != EPassType::Graphics)

				vk::ImageMemoryBarrier2& newBarrier = passImageBarriers.emplace_back();
				newBarrier.srcQueueFamilyIndex = vk::QueueFamilyIgnored;
				newBarrier.dstQueueFamilyIndex = vk::QueueFamilyIgnored;
				newBarrier.subresourceRange = FindSubresourceRange(mTextures[readWrite.GetIndex()]);

				auto foundSrcData = resourceData.find(readWrite);
				TURBO_CHECK_MSG(foundSrcData != resourceData.end(), "Pass tries to read non existent resource")

				FResourceState& srcData = foundSrcData->second;
				newBarrier.srcStageMask = FindSrcStageMask(srcData.mLastUseType);
				newBarrier.dstStageMask = FindDstStageMask(pass.mPassType);

				newBarrier.srcAccessMask = FindSrcAccessMask(srcData.mAccess, pass.mPassType);
				newBarrier.dstAccessMask =
					pass.mPassType == EPassType::Compute
						? vk::AccessFlagBits2::eShaderWrite
						: vk::AccessFlagBits2::eColorAttachmentWrite;

				newBarrier.oldLayout = ToVkImageLayout(srcData.mLayout);
				newBarrier.newLayout = vk::ImageLayout::eGeneral;

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
					// create texture
				}
			}

			// Add barrier
			vk::DependencyInfo dependencyInfo = {};
			dependencyInfo.imageMemoryBarrierCount = mPerPassImageBarriers[passId].size();
			dependencyInfo.pImageMemoryBarriers = mPerPassImageBarriers[passId].data();

			TURBO_CHECK(pass.mExecutePass.IsBound());
			pass.mExecutePass.Execute(gpu, cmd, renderResources);
		}
	}
} // Turbo