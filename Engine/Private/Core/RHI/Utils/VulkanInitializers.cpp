#include "Core/RHI/Utils/VulkanInitializers.h"

#include "Core/Math/Vector.h"
#include "Core/RHI/Utils/VulkanUtils.h"

namespace Turbo {
	vk::CommandPoolCreateInfo VulkanInitializers::CommandPoolCreateInfo(uint32 queueFamilyIndex, vk::CommandPoolCreateFlags flags)
	{
		vk::CommandPoolCreateInfo result{};
		result.setPNext(nullptr);

		result.setQueueFamilyIndex(queueFamilyIndex);
		result.setFlags(flags);

		return result;
	}

	vk::CommandBufferAllocateInfo VulkanInitializers::BufferAllocateInfo(const vk::CommandPool& commandPool, uint32 bufferCount)
	{
		vk::CommandBufferAllocateInfo result{};
		result.setPNext(nullptr);

		result.setCommandPool(commandPool);
		result.setCommandBufferCount(bufferCount);
		result.setLevel(vk::CommandBufferLevel::ePrimary);

		return result;
	}

	vk::CommandBufferBeginInfo VulkanInitializers::BufferBeginInfo(const vk::CommandBufferUsageFlags flags)
	{
		vk::CommandBufferBeginInfo result{};
		result.setPNext(nullptr);

		result.setPInheritanceInfo(nullptr);
		result.setFlags(flags);

		return result;
	}

	vk::FenceCreateInfo VulkanInitializers::FenceCreateInfo(vk::FenceCreateFlags flags)
	{
		vk::FenceCreateInfo result{};
		result.pNext = nullptr;

		result.setFlags(flags);

		return result;
	}

	vk::SemaphoreCreateInfo VulkanInitializers::SemaphoreCreateInfo(vk::SemaphoreCreateFlags flags)
	{
		vk::SemaphoreCreateInfo result{};
		result.pNext = nullptr;

		result.setFlags(flags);

		return result;
	}

	vk::ImageSubresourceRange VulkanInitializers::ImageSubresourceRange(vk::ImageAspectFlags aspectMask)
	{
		vk::ImageSubresourceRange result{};
		result.setAspectMask(aspectMask);
		result.setBaseMipLevel(0);
		result.setLevelCount(vk::RemainingMipLevels);
		result.setBaseArrayLayer(0);
		result.setLayerCount(vk::RemainingArrayLayers);

		return result;
	}

	vk::SemaphoreSubmitInfo VulkanInitializers::SemaphoreSubmitInfo(const vk::Semaphore& semaphore, vk::PipelineStageFlags2 stageMask)
	{
		vk::SemaphoreSubmitInfo result{};
		result.setPNext(nullptr);
		result.setSemaphore(semaphore);
		result.setStageMask(stageMask);
		result.setDeviceIndex(0);
		result.setValue(1);

		return result;
	}

	vk::CommandBufferSubmitInfo VulkanInitializers::CommandBufferSubmitInfo(const vk::CommandBuffer& commandBuffer)
	{
		vk::CommandBufferSubmitInfo result{};
		result.setPNext(nullptr);
		result.setCommandBuffer(commandBuffer);
		result.setDeviceMask(0);

		return result;
	}

	vk::SubmitInfo2 VulkanInitializers::SubmitInfo(const vk::CommandBufferSubmitInfo& commandBuffer, const vk::SemaphoreSubmitInfo* signalSemaphore, const vk::SemaphoreSubmitInfo* waitSemaphore)
	{
		vk::SubmitInfo2 result{};
		result.setPNext(nullptr);

		result.setSignalSemaphoreInfoCount(signalSemaphore ? 1 : 0);
		result.setPSignalSemaphoreInfos(signalSemaphore);
		result.setWaitSemaphoreInfoCount(waitSemaphore ? 1 : 0);
		result.setPWaitSemaphoreInfos(waitSemaphore);

		result.setCommandBufferInfoCount(1);
		result.setPCommandBufferInfos(&commandBuffer);

		return result;
	}

	vk::PresentInfoKHR VulkanInitializers::PresentInfo(const vk::SwapchainKHR& swapChain, const vk::Semaphore& waitSemaphore, const uint32& imageIndex)
	{
		vk::PresentInfoKHR result{};
		result.setPNext(nullptr);

		result.setSwapchains({swapChain});
		result.setWaitSemaphores({waitSemaphore});
		result.setImageIndices({imageIndex});

		return result;
	}

	vk::ImageCreateInfo VulkanInitializers::Image2DCreateInfo(vk::Format format, vk::ImageUsageFlags usageFlags, vk::Extent2D extent, bool bCpuReadback)
	{
		vk::ImageCreateInfo createInfo{};
		createInfo.setPNext(nullptr);

		createInfo.setImageType(vk::ImageType::e2D);

		createInfo.setFormat(format);
		createInfo.setExtent({extent.width, extent.height, 1});

		// No mips and single texture by default
		createInfo.setMipLevels(1);
		createInfo.setArrayLayers(1);

		// Disable multi sampling by default
		createInfo.setSamples(vk::SampleCountFlagBits::e1);

		createInfo.setTiling(bCpuReadback ? vk::ImageTiling::eLinear : vk::ImageTiling::eOptimal);

		createInfo.setUsage(usageFlags);

		return createInfo;
	}

	vk::ImageViewCreateInfo VulkanInitializers::ImageView2DCreateInfo(vk::Format format, const vk::Image& image, vk::ImageAspectFlags imageAspect)
	{
		vk::ImageViewCreateInfo createInfo{};
		createInfo.setPNext(nullptr);

		createInfo.setViewType(vk::ImageViewType::e2D);

		createInfo.setFormat(format);
		createInfo.setImage(image);

		vk::ImageSubresourceRange subresourceRange;
		subresourceRange.setAspectMask(imageAspect);
		subresourceRange.setBaseMipLevel(0);
		subresourceRange.setLevelCount(1);
		subresourceRange.setBaseArrayLayer(0);
		subresourceRange.setLayerCount(1);

		createInfo.setSubresourceRange(subresourceRange);

		return createInfo;
	}

	vk::RenderingAttachmentInfo VulkanInitializers::AttachmentInfo(const vk::ImageView& imageView, vk::ClearValue* clearValue, vk::ImageLayout imageLayout)
	{
		vk::RenderingAttachmentInfo attachmentInfo{};
		attachmentInfo.setImageView(imageView);
		attachmentInfo.setImageLayout(imageLayout);
		attachmentInfo.setLoadOp(clearValue ? vk::AttachmentLoadOp::eClear : vk::AttachmentLoadOp::eLoad);
		attachmentInfo.setStoreOp(vk::AttachmentStoreOp::eStore);

		if (clearValue)
		{
			attachmentInfo.setClearValue(*clearValue);
		}

		return attachmentInfo;
	}

	vk::RenderingInfo VulkanInitializers::RenderingInfo(const glm::uvec2& imgSize, const vk::RenderingAttachmentInfo* colorAttachment, const vk::RenderingAttachmentInfo* depthAttachment, const vk::RenderingAttachmentInfo* stencilAttachment)
	{
		vk::RenderingInfo renderInfo{};
		renderInfo.setRenderArea(vk::Rect2D(vk::Offset2D{0, 0}, VulkanUtils::ToExtent2D(imgSize)));
		renderInfo.setLayerCount(1);
		renderInfo.setColorAttachments({*colorAttachment});
		renderInfo.setPDepthAttachment(depthAttachment);
		renderInfo.setPStencilAttachment(stencilAttachment);

		return renderInfo;
	}
} // Turbo