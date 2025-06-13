#include "Core/RHI/Utils/VulkanInitializers.h"

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
} // Turbo