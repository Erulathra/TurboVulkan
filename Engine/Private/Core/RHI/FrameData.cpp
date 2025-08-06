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
		mSwapChainAcquireSemaphore = rhs.mSwapChainAcquireSemaphore;
		mCommandBuffer = rhs.mCommandBuffer;

		rhs.mRenderFence = nullptr;
		rhs.mSwapChainAcquireSemaphore = nullptr;
		rhs.mCommandBuffer = nullptr;
	}

	void FFrameData::Init()
	{
		const vk::FenceCreateInfo fenceCreateInfo = VulkanInitializers::FenceCreateInfo(vk::FenceCreateFlagBits::eSignaled);
		const vk::SemaphoreCreateInfo semaphoreCreateInfo = VulkanInitializers::SemaphoreCreateInfo();
		const vk::CommandBufferAllocateInfo commandBufferAllocateInfo = VulkanInitializers::BufferAllocateInfo(mDevice->GetRenderCommandPool());

		CHECK_VULKAN_RESULT( mRenderFence, mDevice->Get().createFence(fenceCreateInfo));
		CHECK_VULKAN_RESULT( mSwapChainAcquireSemaphore, mDevice->Get().createSemaphore(semaphoreCreateInfo));

		std::vector<vk::CommandBuffer> commandBuffers;
		CHECK_VULKAN_RESULT( commandBuffers, mDevice->Get().allocateCommandBuffers(commandBufferAllocateInfo));

		mCommandBuffer = commandBuffers[0];
	}

	void FFrameData::Destroy()
	{
		if (mRenderFence)
		{
			mDevice->Get().destroyFence(mRenderFence);
			mRenderFence = nullptr;
		}

		if (mSwapChainAcquireSemaphore)
		{
			mDevice->Get().destroySemaphore(mSwapChainAcquireSemaphore);
		}

		mDeletionQueue.Flush(mDevice);
	}
} // Turbo