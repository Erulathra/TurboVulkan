#include "Graphics/CommandBuffer.h"

#include "Core/Engine.h"
#include "Graphics/GPUDevice.h"
#include "Graphics/Resources.h"
#include "Graphics/VulkanInitializers.h"

namespace Turbo
{
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

	FRenderingAttachments& FRenderingAttachments::Reset()
	{
		mNumColorAttachments = 0;
		mDepthAttachment = {};

		return *this;
	}

	FRenderingAttachments& FRenderingAttachments::AddColorAttachment(THandle<FTexture> textureHandle)
	{
		mColorAttachments[mNumColorAttachments] = textureHandle;
		++mNumColorAttachments;

		return *this;
	}

	FRenderingAttachments& FRenderingAttachments::SetDepthAttachment(THandle<FTexture> textureHandle)
	{
		mDepthAttachment = textureHandle;

		return *this;
	}

	void FCommandBuffer::TransitionImage(THandle<FTexture> textureHandle, vk::ImageLayout newLayout)
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

	void FCommandBuffer::BufferBarrier(THandle<FBuffer> bufferHandle, vk::AccessFlags2 srcAccessMask, vk::PipelineStageFlags2 srcStageMask, vk::AccessFlags2 dstAccessMask, vk::PipelineStageFlags2 dstStageMask)
	{
		const FBuffer* buffer = mGpu->AccessBuffer(bufferHandle);

		vk::BufferMemoryBarrier2 memoryBarrier = {};
		memoryBarrier.srcAccessMask = srcAccessMask;
		memoryBarrier.srcStageMask = srcStageMask;
		memoryBarrier.dstAccessMask = dstAccessMask;
		memoryBarrier.dstStageMask = dstStageMask;
		memoryBarrier.buffer = buffer->mVkBuffer;
		memoryBarrier.offset = 0;
		memoryBarrier.size = vk::WholeSize;

		vk::DependencyInfo dependencyInfo = {};
		dependencyInfo.setBufferMemoryBarriers({memoryBarrier});

		// It's important to insert a buffer memory barrier here to ensure writing to the buffer has finished.
		mVkCommandBuffer.pipelineBarrier2(dependencyInfo);
	}

	void FCommandBuffer::ClearImage(THandle<FTexture> textureHandle, glm::vec4 color)
	{
		TransitionImage(textureHandle, vk::ImageLayout::eGeneral);

		FTexture* texture = mGpu->AccessTexture(textureHandle);
		TURBO_CHECK(texture)

		const vk::ClearColorValue clearColorValue {color.r, color.g, color.b, color.a};
		const vk::ImageSubresourceRange subresourceRange = VkInit::ImageSubresourceRange();
		mVkCommandBuffer.clearColorImage(texture->mImage, vk::ImageLayout::eGeneral, clearColorValue, subresourceRange);
	}

	void FCommandBuffer::BlitImage(THandle<FTexture> src, FRect2DInt srcRect, THandle<FTexture> dst, FRect2DInt dstRect, EFilter filter)
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

	void FCommandBuffer::CopyBuffer(THandle<FBuffer> src, THandle<FBuffer> dst, vk::DeviceSize size)
	{
		vk::BufferCopy2 bufferCopy;
		bufferCopy.srcOffset = 0;
		bufferCopy.dstOffset = 0;
		bufferCopy.size = size;

		const FBuffer* srcBuffer = mGpu->AccessBuffer(src);
		const FBuffer* dstBuffer = mGpu->AccessBuffer(dst);

		vk::CopyBufferInfo2 copyBufferInfo;
		copyBufferInfo.srcBuffer = srcBuffer->mVkBuffer;
		copyBufferInfo.dstBuffer = dstBuffer->mVkBuffer;
		copyBufferInfo.setRegions(bufferCopy);

		mVkCommandBuffer.copyBuffer2(copyBufferInfo);
	}

	void FCommandBuffer::BindDescriptorSet(THandle<FDescriptorSet> descriptorSetHandle, uint32 setIndex)
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

	void FCommandBuffer::BindPipeline(THandle<FPipeline> pipelineHandle)
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

	void FCommandBuffer::BeginRendering(const FRenderingAttachments& renderingAttachments)
	{
		TURBO_CHECK(renderingAttachments.mNumColorAttachments > 0)

		vk::RenderingInfo renderingInfo = {};
		renderingInfo.layerCount = 1;

		std::array<vk::RenderingAttachmentInfo, 8> colorAttachments;
		vk::RenderingAttachmentInfo depthAttachmentInfo;

		glm::ivec2 attachmentSize;

		for (uint32 attachmentIndex = 0; attachmentIndex < renderingAttachments.mNumColorAttachments; ++attachmentIndex)
		{
			const FTexture* attachmentTexture = mGpu->AccessTexture(renderingAttachments.mColorAttachments[attachmentIndex]);
			TURBO_CHECK(attachmentTexture)

			TURBO_CHECK(attachmentIndex == 0 || attachmentSize == attachmentTexture->GetSize2D())
			attachmentSize = attachmentTexture->GetSize2D();

			vk::RenderingAttachmentInfo& attachmentInfo = colorAttachments[attachmentIndex];
			attachmentInfo = vk::RenderingAttachmentInfo();
			attachmentInfo.imageView = attachmentTexture->mImageView;
			attachmentInfo.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
			attachmentInfo.loadOp = vk::AttachmentLoadOp::eLoad;
			attachmentInfo.storeOp = vk::AttachmentStoreOp::eStore;
		}

		renderingInfo.renderArea.extent = VulkanConverters::ToExtent2D(attachmentSize);
		renderingInfo.pColorAttachments = colorAttachments.data();
		renderingInfo.colorAttachmentCount = renderingAttachments.mNumColorAttachments;

		if (renderingAttachments.mDepthAttachment.IsValid())
		{
			const FTexture* depthTexture = mGpu->AccessTexture(renderingAttachments.mDepthAttachment);
			TURBO_CHECK(depthTexture)

			TURBO_CHECK(attachmentSize == depthTexture->GetSize2D())
			depthAttachmentInfo.imageView = depthTexture->mImageView;
			depthAttachmentInfo.imageLayout = vk::ImageLayout::eDepthAttachmentOptimal;
			depthAttachmentInfo.loadOp = vk::AttachmentLoadOp::eClear;
			depthAttachmentInfo.storeOp = vk::AttachmentStoreOp::eStore;
			depthAttachmentInfo.clearValue.depthStencil.depth = 0.f;

			renderingInfo.pDepthAttachment = &depthAttachmentInfo;
		}

		mVkCommandBuffer.beginRendering(renderingInfo);
	}

	void FCommandBuffer::EndRendering()
	{
		mVkCommandBuffer.endRendering();
	}

	void FCommandBuffer::SetViewport(const FViewport& viewport)
	{
		vk::Viewport vkViewport = {};
		vkViewport.x = viewport.Rect.Position.x;
		vkViewport.y = viewport.Rect.Position.y;
		vkViewport.width = viewport.Rect.Size.x;
		vkViewport.height = viewport.Rect.Size.y;
		vkViewport.minDepth = viewport.MinDepth;
		vkViewport.maxDepth = viewport.MaxDepth;

		mVkCommandBuffer.setViewport(0, 1, &vkViewport);
	}

	void FCommandBuffer::SetScissor(const FRect2DInt& rect)
	{
		vk::Rect2D vkScissor = {};
		vkScissor.offset = VulkanConverters::ToOffset2D(rect.Position);
		vkScissor.extent = VulkanConverters::ToExtent2D(rect.Size);

		mVkCommandBuffer.setScissor(0, 1, &vkScissor);
	}

	void FCommandBuffer::Draw(uint32 vertexCount, uint32 instanceCount, uint32 firstVertex, uint32 firstInstance)
	{
		mVkCommandBuffer.draw(vertexCount, instanceCount, firstVertex, firstInstance);
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
			stageFlagBits = vk::ShaderStageFlagBits::eAllGraphics;
		}

		mVkCommandBuffer.pushConstants(currentPipeline->mVkLayout, stageFlagBits, 0, size, pushConstants);
	}
} // Turbo