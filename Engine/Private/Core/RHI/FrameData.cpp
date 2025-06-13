#include "Core/RHI/FrameData.h"

#include "Core/RHI/VulkanDevice.h"

namespace Turbo {
	FFrameData::FFrameData(FVulkanDevice& device)
		: mDevice(&device)
	{
	}

	FFrameData::FFrameData(FFrameData&& rhs) noexcept
	{
		mDevice = rhs.mDevice;

		mRenderFence = rhs.mRenderFence;
		mSwapChainSemaphore = rhs.mSwapChainSemaphore;
		mRenderSemaphore = rhs.mRenderSemaphore;
		mCommandBuffer = rhs.mCommandBuffer;

		rhs.mRenderFence = nullptr;
		rhs.mSwapChainSemaphore = nullptr;
		rhs.mRenderSemaphore = nullptr;
		rhs.mCommandBuffer = nullptr;
	}

	void FFrameData::Init()
	{
		const vk::FenceCreateInfo fenceCreateInfo = VulkanInitializers::FenceCreateInfo(vk::FenceCreateFlagBits::eSignaled);
		const vk::SemaphoreCreateInfo semaphoreCreateInfo = VulkanInitializers::SemaphoreCreateInfo();
		const vk::CommandBufferAllocateInfo commandBufferAllocateInfo = VulkanInitializers::BufferAllocateInfo(mDevice->GetRenderCommandPool());

		vk::Result vkResult;
		std::tie(vkResult, mRenderFence) = mDevice->Get().createFence(fenceCreateInfo);
		CHECK_VULKAN_HPP(vkResult);
		std::tie(vkResult,mSwapChainSemaphore) = mDevice->Get().createSemaphore(semaphoreCreateInfo);
		CHECK_VULKAN_HPP(vkResult);
		std::tie(vkResult, mRenderSemaphore) = mDevice->Get().createSemaphore(semaphoreCreateInfo);
		CHECK_VULKAN_HPP(vkResult);

		std::vector<vk::CommandBuffer> commandBuffers;
		std::tie(vkResult, commandBuffers) = mDevice->Get().allocateCommandBuffers(commandBufferAllocateInfo);
		CHECK_VULKAN_HPP(vkResult);

		mCommandBuffer = commandBuffers[0];
	}

	void FFrameData::Destroy()
	{
		if (mRenderFence)
		{
			mDevice->Get().destroyFence(mRenderFence);
			mRenderFence = nullptr;
		}

		if (mSwapChainSemaphore)
		{
			mDevice->Get().destroySemaphore(mSwapChainSemaphore);
			mSwapChainSemaphore = nullptr;
		}

		if (mRenderSemaphore)
		{
			mDevice->Get().destroySemaphore(mRenderSemaphore);
			mRenderSemaphore = nullptr;
		}
	}
} // Turbo