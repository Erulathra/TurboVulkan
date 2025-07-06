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
		mCMD = rhs.mCMD;

		rhs.mRenderFence = nullptr;
		rhs.mSwapChainSemaphore = nullptr;
		rhs.mRenderSemaphore = nullptr;
		rhs.mCMD = nullptr;
	}

	void FFrameData::Init()
	{
		const vk::FenceCreateInfo fenceCreateInfo = VulkanInitializers::FenceCreateInfo(vk::FenceCreateFlagBits::eSignaled);
		const vk::SemaphoreCreateInfo semaphoreCreateInfo = VulkanInitializers::SemaphoreCreateInfo();
		const vk::CommandBufferAllocateInfo commandBufferAllocateInfo = VulkanInitializers::BufferAllocateInfo(mDevice->GetRenderCommandPool());

		CHECK_VULKAN_RESULT( mRenderFence, mDevice->Get().createFence(fenceCreateInfo));
		CHECK_VULKAN_RESULT( mSwapChainSemaphore, mDevice->Get().createSemaphore(semaphoreCreateInfo));
		CHECK_VULKAN_RESULT( mRenderSemaphore, mDevice->Get().createSemaphore(semaphoreCreateInfo));

		std::vector<vk::CommandBuffer> commandBuffers;
		CHECK_VULKAN_RESULT( commandBuffers, mDevice->Get().allocateCommandBuffers(commandBufferAllocateInfo));

		mCMD = commandBuffers[0];
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

		mDeletionQueue.Flush(mDevice);
	}
} // Turbo