#include "Core/RHI/Utils/VulkanUtils.h"

#include "Core/RHI/VulkanDevice.h"

namespace Turbo {
	void VulkanUtils::TransitionImage(
		const vk::CommandBuffer& cmd,
		const vk::Image& image,
		const vk::ImageLayout oldLayout,
		const vk::ImageLayout newLayout,
		const vk::ImageAspectFlags aspectMask
		)
	{
		vk::ImageMemoryBarrier2 imageBarrier{};
		imageBarrier.setPNext(nullptr);

		imageBarrier.setSrcStageMask(vk::PipelineStageFlagBits2::eAllCommands);
		imageBarrier.setSrcAccessMask(vk::AccessFlagBits2::eMemoryWrite);
		imageBarrier.setDstStageMask(vk::PipelineStageFlagBits2::eAllCommands);
		imageBarrier.setDstAccessMask(vk::AccessFlagBits2::eMemoryWrite | vk::AccessFlagBits2::eMemoryRead);

		imageBarrier.setOldLayout(oldLayout);
		imageBarrier.setNewLayout(newLayout);

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

	vk::Viewport VulkanUtils::CreateViewport(const glm::vec2& start, const glm::vec2& end, float depthStart, float depthEnd)
	{
		vk::Viewport result;
		result.setX(start.x);
		result.setY(start.y);
		const glm::vec2 size = glm::abs(end - start);
		result.setWidth(size.x);
		result.setHeight(size.y);
		result.setMinDepth(depthStart);
		result.setMaxDepth(depthEnd);

		return result;
	}

	vk::Rect2D VulkanUtils::CreateRect(const glm::ivec2& start, const glm::ivec2& end)
	{
		vk::Rect2D result;
		result.setOffset(ToOffset2D(start));
		const glm::ivec2 size = glm::abs(end - start);
		result.setExtent(ToExtent2D(size));

		return result;
	}

	vk::ShaderModule VulkanUtils::CreateShaderModule(const FVulkanDevice* device, const std::span<uint32>& shaderData)
	{
		vk::ShaderModuleCreateInfo createInfo {};
		createInfo.setCode(shaderData);

		vk::ShaderModule shaderModule = nullptr;
		vk::Result result;
		std::tie(result, shaderModule) = device->Get().createShaderModule(createInfo);

		if (result == vk::Result::eSuccess)
		{
			return shaderModule;
		}

		return nullptr;
	}
} // Turbo