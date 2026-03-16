#include "Graphics/GPUDevice.h"
#include "Graphics/VulkanHelpers.h"
#include "Graphics/VulkanInitializers.h"
#include "Graphics/FrameGraph/RenderGraph.h"

namespace Turbo
{
	vk::ImageSubresourceRange FindSubresourceRange(vk::Format format)
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

	bool FRGTextureInfo::IsValid() const
	{
		return mWidth * mHeight > 1
			&& mFormat != vk::Format::eUndefined
			&& mName.IsNone() == false;
	}

	bool FRGBufferInfo::IsValid() const
	{
		return mBufferFlags != EBufferFlags::None
			&& mSize > 0;
	}

	vk::BufferMemoryBarrier2 FRGBufferMemoryBarrier::ToVkBufferBarrier(FGPUDevice& gpu, THandle<FBuffer> bufferHandle) const
	{
		vk::BufferMemoryBarrier2 vkBarrier = {};
		vkBarrier.srcStageMask = mSrcStageMask;
		vkBarrier.srcAccessMask = mSrcAccessMask;
		vkBarrier.dstStageMask = mDstStageMask;
		vkBarrier.dstAccessMask = mDstAccessMask;
		vkBarrier.srcQueueFamilyIndex = vk::QueueFamilyIgnored;
		vkBarrier.dstQueueFamilyIndex = vk::QueueFamilyIgnored;

		const FBuffer* buffer = gpu.AccessBuffer(bufferHandle);
		vkBarrier.buffer = buffer->mVkBuffer;
		vkBarrier.offset = 0;
		vkBarrier.size = vk::WholeSize;

		return vkBarrier;
	}

	vk::ImageMemoryBarrier2 FRGTextureMemoryBarrier::ToVkImageBarrier(FGPUDevice& gpu, THandle<FTexture> textureHandle) const
	{
		vk::ImageMemoryBarrier2 vkBarrier = {};
		vkBarrier.srcStageMask = mSrcStageMask;
		vkBarrier.srcAccessMask = mSrcAccessMask;
		vkBarrier.dstStageMask = mDstStageMask;
		vkBarrier.dstAccessMask = mDstAccessMask;
		vkBarrier.oldLayout = ToVkImageLayout(mOldLayout);
		vkBarrier.newLayout = ToVkImageLayout(mNewLayout);
		vkBarrier.srcQueueFamilyIndex = vk::QueueFamilyIgnored;
		vkBarrier.dstQueueFamilyIndex = vk::QueueFamilyIgnored;

		const FTexture* texture = gpu.AccessTexture(textureHandle);
		const FTextureCold* textureCold = gpu.AccessTextureCold(textureHandle);
		TURBO_CHECK(texture)

		vkBarrier.image = texture->mVkImage;
		vkBarrier.subresourceRange = FindSubresourceRange(textureCold->mFormat);

		return vkBarrier;
	}
}
