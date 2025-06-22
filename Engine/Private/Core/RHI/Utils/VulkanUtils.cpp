#include "Core/RHI/Utils/VulkanUtils.h"

#include "Core/Math/Math.h"
#include "Core/Math/Math.h"
#include "Core/Math/Math.h"
#include "Core/Math/Math.h"
#include "Core/Math/Math.h"
#include "Core/Math/Math.h"
#include "Core/Math/Math.h"
#include "Core/Math/Math.h"

namespace Turbo {
	void VulkanUtils::TransitionImage(
		const vk::CommandBuffer& cmd,
		const vk::Image& image,
		const vk::ImageLayout oldLayout,
		const vk::ImageLayout newLayout)
	{
		vk::ImageMemoryBarrier2 imageBarrier{};
		imageBarrier.setPNext(nullptr);

		imageBarrier.setSrcStageMask(vk::PipelineStageFlagBits2::eAllCommands);
		imageBarrier.setSrcAccessMask(vk::AccessFlagBits2::eMemoryWrite);
		imageBarrier.setDstStageMask(vk::PipelineStageFlagBits2::eAllCommands);
		imageBarrier.setDstAccessMask(vk::AccessFlagBits2::eMemoryWrite | vk::AccessFlagBits2::eMemoryRead);

		imageBarrier.setOldLayout(oldLayout);
		imageBarrier.setNewLayout(newLayout);

		const vk::ImageAspectFlags aspectMask =
			newLayout == vk::ImageLayout::eColorAttachmentOptimal
				? vk::ImageAspectFlagBits::eDepth
				: vk::ImageAspectFlagBits::eColor;

		imageBarrier.setSubresourceRange(VulkanInitializers::ImageSubresourceRange(aspectMask));
		imageBarrier.setImage(image);

		vk::DependencyInfo dependencyInfo{};
		dependencyInfo.setPNext(nullptr);

		dependencyInfo.setImageMemoryBarrierCount(1);
		dependencyInfo.setPImageMemoryBarriers(&imageBarrier);

		cmd.pipelineBarrier2(dependencyInfo);
	}

	void VulkanUtils::BlitImage(const vk::CommandBuffer& cmd, const vk::Image& src, const glm::uvec2& srcSize, const vk::Image& dst, const glm::uvec2& dstSize)
	{
		vk::ImageBlit2 region{};
		region.setPNext(nullptr);

		region.srcOffsets[0] = ToOffset3D(glm::ivec3(0));
		region.srcOffsets[1] = ToOffset3D(glm::ivec3(srcSize, 1));
		region.dstOffsets[0] = ToOffset3D(glm::ivec3(0));
		region.dstOffsets[1] = ToOffset3D(glm::ivec3(dstSize, 1));

		vk::ImageSubresourceLayers layers{};
		layers.setAspectMask(vk::ImageAspectFlagBits::eColor);
		layers.setBaseArrayLayer(0);
		layers.setLayerCount(1);
		layers.setMipLevel(0);
		region.setSrcSubresource(layers);
		region.setDstSubresource(layers);

		vk::BlitImageInfo2 blitInfo{};
		blitInfo.setPNext(nullptr);

		blitInfo.setSrcImage(src);
		blitInfo.setSrcImageLayout(vk::ImageLayout::eTransferSrcOptimal);
		blitInfo.setDstImage(dst);
		blitInfo.setDstImageLayout(vk::ImageLayout::eTransferDstOptimal);

		blitInfo.setFilter(vk::Filter::eLinear);
		blitInfo.setRegionCount(1);
		blitInfo.setRegions(region);

		cmd.blitImage2(blitInfo);
	}
} // Turbo