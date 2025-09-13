#include "Graphics/CommandBuffer.h"

#include "Core/Engine.h"
#include "Graphics/GPUDevice.h"
#include "Graphics/Resources.h"
#include "Graphics/VulkanInitializers.h"

namespace Turbo
{
	void FCommandBuffer::BeginRendering(FTextureHandle renderTargetHandle)
	{
		FTexture* renderTarget = mGpu->AccessTexture(renderTargetHandle);
		TURBO_CHECK(renderTarget)

		vk::RenderingAttachmentInfo colorAttachment = {};
		colorAttachment.imageView = renderTarget->mImageView;
		colorAttachment.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
		colorAttachment.loadOp = vk::AttachmentLoadOp::eLoad;

		vk::RenderingInfo renderInfo = {};
		renderInfo.layerCount = 1;
		renderInfo.colorAttachmentCount = 1;
		renderInfo.pColorAttachments = &colorAttachment;

		renderInfo.renderArea = vk::Rect2D(vk::Offset2D(0.f, 0.f), VulkanConverters::ToExtent2D(renderTarget->GetSize()));

		mVkCommandBuffer.beginRendering(renderInfo);
	}

	void FCommandBuffer::EndRendering()
	{
		mVkCommandBuffer.endRendering();
	}

	void FCommandBuffer::Begin()
	{
		mbRecording = true;
		vk::CommandBufferBeginInfo beginInfo = VkInit::BufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
		CHECK_VULKAN_HPP(mVkCommandBuffer.begin(beginInfo));
	}

	void FCommandBuffer::End()
	{
		CHECK_VULKAN_HPP(mVkCommandBuffer.end())
	}

	void FCommandBuffer::TransitionImage(FTextureHandle textureHandle, vk::ImageLayout newLayout)
	{
		FTexture* texture = mGpu->AccessTexture(textureHandle);
		TURBO_CHECK(texture)

		if (texture->mCurrentLayout == newLayout)
		{
			return;
		}

		vk::ImageMemoryBarrier2 imageBarrier{};
		imageBarrier.setSrcStageMask(vk::PipelineStageFlagBits2::eAllCommands);
		imageBarrier.setSrcAccessMask(vk::AccessFlagBits2::eMemoryWrite);
		imageBarrier.setDstStageMask(vk::PipelineStageFlagBits2::eAllCommands);
		imageBarrier.setDstAccessMask(vk::AccessFlagBits2::eMemoryWrite | vk::AccessFlagBits2::eMemoryRead);

		imageBarrier.setOldLayout(texture->mCurrentLayout);
		imageBarrier.setNewLayout(newLayout);

		const vk::ImageAspectFlags aspectFlags = newLayout == vk::ImageLayout::eDepthAttachmentOptimal ? vk::ImageAspectFlagBits::eDepth : vk::ImageAspectFlagBits::eColor;
		imageBarrier.setSubresourceRange(VkInit::ImageSubresourceRange(aspectFlags));
		imageBarrier.setImage(texture->mImage);

		vk::DependencyInfo dependencyInfo{};
		dependencyInfo.setImageMemoryBarrierCount(1);
		dependencyInfo.setPImageMemoryBarriers(&imageBarrier);

		mVkCommandBuffer.pipelineBarrier2(dependencyInfo);
		texture->mCurrentLayout = newLayout;
	}

	void FCommandBuffer::ClearImage(FTextureHandle textureHandle, glm::vec4 color)
	{
		TransitionImage(textureHandle, vk::ImageLayout::eGeneral);

		FTexture* texture = mGpu->AccessTexture(textureHandle);
		TURBO_CHECK(texture)

		const vk::ClearColorValue clearColorValue {color.r, color.g, color.b, color.a};
		const vk::ImageSubresourceRange subresourceRange = VkInit::ImageSubresourceRange();
		mVkCommandBuffer.clearColorImage(texture->mImage, vk::ImageLayout::eGeneral, clearColorValue, subresourceRange);
	}

	void FCommandBuffer::BlitImage(FTextureHandle src, FRect2DInt srcRect, FTextureHandle dst, FRect2DInt dstRect, EFilter filter)
	{
		vk::ImageBlit2 region = {};
		region.setPNext(nullptr);

		region.srcOffsets[0] = VulkanConverters::ToOffset3D(glm::ivec3(srcRect.Position, 0.f));
		region.srcOffsets[1] = VulkanConverters::ToOffset3D(glm::ivec3(srcRect.Position + srcRect.Size, 1.f));

		region.dstOffsets[0] = VulkanConverters::ToOffset3D(glm::ivec3(dstRect.Position, 0.f));
		region.dstOffsets[1] = VulkanConverters::ToOffset3D(glm::ivec3(dstRect.Position + dstRect.Size, 1.f));

		vk::ImageSubresourceLayers layers = {};
		layers.aspectMask = vk::ImageAspectFlagBits::eColor;
		layers.baseArrayLayer = 0;
		layers.layerCount = 1;
		layers.mipLevel = 0;
		region.setSrcSubresource(layers);
		region.setDstSubresource(layers);

		vk::BlitImageInfo2 blitInfo{};
		blitInfo.setPNext(nullptr);

		FTexture* srcTexture = mGpu->AccessTexture(src);
		FTexture* dstTexture = mGpu->AccessTexture(dst);
		TURBO_CHECK(srcTexture && dstTexture)

		blitInfo.srcImage = srcTexture->mImage;
		blitInfo.srcImageLayout = vk::ImageLayout::eTransferSrcOptimal;
		blitInfo.dstImage = dstTexture->mImage;
		blitInfo.dstImageLayout = vk::ImageLayout::eTransferDstOptimal;

		blitInfo.filter = VkConvert::ToVkFilter(filter);
		blitInfo.setRegions({region});

		mVkCommandBuffer.blitImage2(blitInfo);
	}

	void FCommandBuffer::BindDescriptorSet(FDescriptorSetHandle descriptorSetHandle, uint32 setIndex)
	{
		const FDescriptorSet* descriptorSet = mGpu->AccessDescriptorSet(descriptorSetHandle);
		const FPipeline* currentPipeline = mGpu->AccessPipeline(mCurrentPipeline);

		mVkCommandBuffer.bindDescriptorSets(
			currentPipeline->mVkBindPoint,
			currentPipeline->mVkLayout,
			setIndex,
			1,
			&descriptorSet->mVkDescriptorSet,
			0,
			nullptr
		);
	}

	void FCommandBuffer::BindPipeline(FPipelineHandle pipelineHandle)
	{
		const FPipeline* pipeline = mGpu->AccessPipeline(pipelineHandle);
		TURBO_CHECK(pipeline);

		mVkCommandBuffer.bindPipeline(pipeline->mVkBindPoint, pipeline->mVkPipeline);
		mCurrentPipeline = pipelineHandle;
	}

	void FCommandBuffer::Dispatch(const glm::ivec3& groupCount)
	{
		mVkCommandBuffer.dispatch(groupCount.x, groupCount.y, groupCount.z);
	}

	void FCommandBuffer::Reset()
	{
		for (int setId = 0; setId < kMaxDescriptorSets; ++setId)
		{
			mBoundDescriptorSets[setId].Reset();
		}
		mCurrentPipeline.Reset();
		mbRecording = false;
	}

	vk::CommandBufferSubmitInfo FCommandBuffer::CreateSubmitInfo() const
	{
		vk::CommandBufferSubmitInfo result{};
		result.setPNext(nullptr);
		result.setCommandBuffer(mVkCommandBuffer);
		result.setDeviceMask(0);

		return result;
	}

	void FCommandBuffer::PushConstantsImpl(void* pushConstants, uint32 size)
	{
		const FPipeline* currentPipeline = mGpu->AccessPipeline(mCurrentPipeline);

		vk::ShaderStageFlags stageFlagBits = vk::ShaderStageFlagBits::eCompute;
		if (currentPipeline->mbGraphicsPipeline)
		{
			TURBO_UNINPLEMENTED()
		}

		mVkCommandBuffer.pushConstants(currentPipeline->mVkLayout, stageFlagBits, 0, size, pushConstants);
	}
} // Turbo