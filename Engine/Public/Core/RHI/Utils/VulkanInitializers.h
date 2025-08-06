#pragma once

#include "../RHICore.h"

namespace Turbo {

namespace VulkanInitializers {
	vk::CommandPoolCreateInfo CommandPoolCreateInfo(uint32 queueFamilyIndex, vk::CommandPoolCreateFlags flags = {});
	vk::CommandBufferAllocateInfo BufferAllocateInfo(const vk::CommandPool& commandPool, uint32 bufferCount = 1);
	vk::CommandBufferBeginInfo BufferBeginInfo(const vk::CommandBufferUsageFlags flags = {});

	vk::FenceCreateInfo FenceCreateInfo(vk::FenceCreateFlags flags = {});
	vk::SemaphoreCreateInfo SemaphoreCreateInfo(vk::SemaphoreCreateFlags flags = {});

	vk::ImageSubresourceRange ImageSubresourceRange(vk::ImageAspectFlags aspectMask = vk::ImageAspectFlagBits::eColor);

	vk::SemaphoreSubmitInfo SemaphoreSubmitInfo(const vk::Semaphore& signalSemaphore, vk::PipelineStageFlags2 stageMask);
	vk::CommandBufferSubmitInfo CommandBufferSubmitInfo(const vk::CommandBuffer& commandBuffer);
	vk::SubmitInfo2 SubmitInfo(const vk::CommandBufferSubmitInfo& commandBuffer, const vk::SemaphoreSubmitInfo* signalSemaphore, const vk::SemaphoreSubmitInfo* waitSemaphore);

	vk::PresentInfoKHR PresentInfo(const vk::SwapchainKHR& swapChain, const vk::Semaphore& waitSemaphore, const uint32& imageIndex);

	vk::ImageCreateInfo Image2DCreateInfo(vk::Format format, vk::ImageUsageFlags usageFlags, vk::Extent2D extent, bool bCpuReadback = false);
	vk::ImageViewCreateInfo ImageView2DCreateInfo(vk::Format format, const vk::Image& image, vk::ImageAspectFlags imageAspect);

	vk::RenderingAttachmentInfo AttachmentInfo(const vk::ImageView& imageView, vk::ClearValue* clearValue = nullptr, vk::ImageLayout imageLayout = vk::ImageLayout::eAttachmentOptimal);
	vk::RenderingInfo RenderingInfo(const glm::uvec2& imgSize, const vk::RenderingAttachmentInfo* colorAttachment, const vk::RenderingAttachmentInfo* depthAttachment = nullptr, const vk::RenderingAttachmentInfo* stencilAttachment = nullptr);
};

} // Turbo
