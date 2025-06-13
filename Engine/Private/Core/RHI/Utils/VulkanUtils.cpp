#include "Core/RHI/Utils/VulkanUtils.h"

namespace Turbo {
	void VulkanUtils::TransitionImage(
		const vk::CommandBuffer& commandBuffer,
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

		commandBuffer.pipelineBarrier2(dependencyInfo);
	}
} // Turbo