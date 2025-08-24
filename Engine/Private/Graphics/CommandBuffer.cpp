#include "Graphics/CommandBuffer.h"

#include "Core/Engine.h"
#include "Graphics/GPUDevice.h"
#include "Graphics/Resources.h"

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

	void FCommandBuffer::TransitionImage(FTextureHandle textureHandle, vk::ImageLayout newLayout)
	{
		FTexture* texture = mDevice->AccessTexture(textureHandle);
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

		imageBarrier.setSubresourceRange(VkInit::ImageSubresourceRange(texture->mAspectFlags));
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

		FTexture* texture = mDevice->AccessTexture(textureHandle);
		TURBO_CHECK(texture)

		const vk::ClearColorValue clearColorValue {color.r, color.g, color.b, color.a};
		const vk::ImageSubresourceRange subresourceRange = VkInit::ImageSubresourceRange();
		mVkCommandBuffer.clearColorImage(texture->mImage, vk::ImageLayout::eGeneral, clearColorValue, subresourceRange);
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
} // Turbo