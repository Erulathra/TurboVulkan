#pragma once

#include "Graphics/GraphicsCore.h"

namespace Turbo {
	namespace VulkanInitializers
	{
		[[nodiscard]] vk::CommandPoolCreateInfo CommandPoolCreateInfo(uint32 queueFamilyIndex, vk::CommandPoolCreateFlags flags = {});
		[[nodiscard]] vk::CommandBufferAllocateInfo BufferAllocateInfo(const vk::CommandPool& commandPool, uint32 bufferCount = 1);

		[[nodiscard]] vk::FenceCreateInfo FenceCreateInfo(vk::FenceCreateFlags flags = {});
		[[nodiscard]] vk::SemaphoreCreateInfo SemaphoreCreateInfo(vk::SemaphoreCreateFlags flags = {});

		[[nodiscard]] vk::ImageSubresourceRange ImageSubresourceRange(vk::ImageAspectFlags aspectMask = vk::ImageAspectFlagBits::eColor);

		[[nodiscard]] vk::SemaphoreSubmitInfo SemaphoreSubmitInfo(vk::Semaphore signalSemaphore, vk::PipelineStageFlags2 stageMask);
		[[nodiscard]] vk::CommandBufferSubmitInfo CommandBufferSubmitInfo(vk::CommandBuffer commandBuffer);
		[[nodiscard]] vk::SubmitInfo2 SubmitInfo(const vk::CommandBufferSubmitInfo& commandBuffer, const vk::SemaphoreSubmitInfo* signalSemaphore, const vk::SemaphoreSubmitInfo* waitSemaphore);

		[[nodiscard]] vk::PresentInfoKHR PresentInfo(const vk::SwapchainKHR& swapChain, const vk::Semaphore& waitSemaphore, const uint32& imageIndex);

		[[nodiscard]] vk::ImageCreateInfo Image2DCreateInfo(vk::Format format, vk::ImageUsageFlags usageFlags, vk::Extent2D extent, bool bCpuReadback = false);
		[[nodiscard]] vk::ImageViewCreateInfo ImageView2DCreateInfo(vk::Format format, const vk::Image& image, vk::ImageAspectFlags imageAspect);

		[[nodiscard]] vk::RenderingAttachmentInfo AttachmentInfo(const vk::ImageView& imageView, vk::ClearValue* clearValue = nullptr, vk::ImageLayout imageLayout = vk::ImageLayout::eAttachmentOptimal);
		[[nodiscard]] vk::RenderingInfo RenderingInfo(const glm::uvec2& imgSize, const vk::RenderingAttachmentInfo* colorAttachment, const vk::RenderingAttachmentInfo* depthAttachment = nullptr, const vk::RenderingAttachmentInfo* stencilAttachment = nullptr);
	};

	namespace VkInit = VulkanInitializers;
} // Turbo
