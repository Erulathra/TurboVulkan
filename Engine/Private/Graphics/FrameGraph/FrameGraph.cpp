#include "Graphics/FrameGraph/FrameGraph.h"

namespace Turbo
{
	FRGResourceHandle FRGPassInfo::CreateTexture(const FRGTextureInfo& textureInfo)
	{
		mGraphBuilder->mTextures.push_back(textureInfo);
		FRGResourceHandle textureHandle(ERGResourceType::Texture, mGraphBuilder->mTextures.size() - 1);
		mTextureCreations.push_back(textureHandle);

		return textureHandle;
	}

	FRGResourceHandle FRGPassInfo::ReadTexture(FRGResourceHandle texture, vk::ImageLayout targetLayout)
	{
		mTextureReads.emplace_back(texture, targetLayout);
		return texture;
	}

	FRGResourceHandle FRGPassInfo::WriteTexture(FRGResourceHandle texture)
	{
		texture = texture.IncrementVersion();
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

	FRGResourceHandle FRGPassInfo::CreateBuffer(const FRGBufferInfo& bufferInfo)
	{
		mGraphBuilder->mBuffers.push_back(bufferInfo);
		FRGResourceHandle bufferHandle(ERGResourceType::Buffer, mGraphBuilder->mBuffers.size() - 1);

		WriteBuffer(bufferHandle);

		return bufferHandle;
	}

	FRGResourceHandle FRGPassInfo::ReadBuffer(FRGResourceHandle handle)
	{
		mBufferReads.push_back(handle);
		return handle;
	}

	FRGResourceHandle FRGPassInfo::WriteBuffer(FRGResourceHandle handle)
	{
		handle = handle.IncrementVersion();
		mBufferWrites.push_back(handle);

		return handle;
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

	void FRenderGraphBuilder::Compile()
	{
		TRACE_ZONE_SCOPED()

		CompileSynchronization();
	}

	void FRenderGraphBuilder::Run()
	{
		TRACE_ZONE_SCOPED()
	}

	enum class ERGResourceUsageType
	{
		Create,
		Read,
		Write
	};

	struct FRGResourceTracking
	{
		FRGPassHandle mPreviousPass = {};
		ERGResourceUsageType mPreviousUsage = ERGResourceUsageType::Create;
		vk::ImageLayout mPreviousLayout = vk::ImageLayout::eUndefined;

		FRGPassHandle mPass = {};
		ERGResourceUsageType mUsage = ERGResourceUsageType::Create;
		vk::ImageLayout mLayout = vk::ImageLayout::eUndefined;
	};

	void FRenderGraphBuilder::CompileSynchronization()
	{
		entt::dense_map<FRGResourceHandle>

	}
} // Turbo