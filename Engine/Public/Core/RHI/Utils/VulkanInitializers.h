#pragma once

#include "../RHICore.h"

namespace Turbo {

class VulkanInitializers {
public:
	static vk::CommandPoolCreateInfo CommandPoolCreateInfo(uint32 queueFamilyIndex, vk::CommandPoolCreateFlags flags = {});
	static vk::CommandBufferAllocateInfo BufferAllocateInfo(const vk::CommandPool& commandPool, uint32 bufferCount = 1);
	static vk::CommandBufferBeginInfo BufferBeginInfo(const vk::CommandBufferUsageFlags flags = {});

	static vk::FenceCreateInfo FenceCreateInfo(vk::FenceCreateFlags flags = {});
	static vk::SemaphoreCreateInfo SemaphoreCreateInfo(vk::SemaphoreCreateFlags flags = {});

	static vk::ImageSubresourceRange ImageSubresourceRange(vk::ImageAspectFlags aspectMask = vk::ImageAspectFlagBits::eColor);

	static vk::SemaphoreSubmitInfo SemaphoreSubmitInfo(const vk::Semaphore& semaphore, vk::PipelineStageFlags2 stageMask);
	static vk::CommandBufferSubmitInfo CommandBufferSubmitInfo(const vk::CommandBuffer& commandBuffer);
	static vk::SubmitInfo2 SubmitInfo(const vk::CommandBufferSubmitInfo& commandBuffer, const vk::SemaphoreSubmitInfo* signalSemaphore, const vk::SemaphoreSubmitInfo* waitSemaphore);

	static vk::PresentInfoKHR PresentInfo(const vk::SwapchainKHR& swapChain, const vk::Semaphore& waitSemaphore, const uint32& imageIndex);

	static vk::ImageCreateInfo Image2DCreateInfo(vk::Format format, vk::ImageUsageFlags usageFlags, vk::Extent2D extent, bool bCpuReadback = false);
	static vk::ImageViewCreateInfo ImageView2DCreateInfo(vk::Format format, const vk::Image& image, vk::ImageAspectFlags imageAspect);
};

} // Turbo
