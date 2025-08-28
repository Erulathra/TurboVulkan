#pragma once

#include "RHICore.h"
#include "../../Graphics/DestoryQueue.h"

namespace Turbo {
	class FVulkanDevice;

	class FFrameData
	{

	public:
		FFrameData() = delete;

		// Disallow copy
		FFrameData(const FFrameData&) = delete;
		FFrameData& operator=(const FFrameData&) = delete;

		explicit FFrameData(FVulkanDevice& device);
		FFrameData(FFrameData&& rhs) noexcept;

	public:
		void Init();
		void Destroy();

		vk::CommandBuffer& GetCommandBuffer() { return mCommandBuffer; }
		FDestroyQueue& GetDeletionQueue() { return mDeletionQueue; }

	private:
		FVulkanDevice* mDevice;

		vk::CommandBuffer mCommandBuffer;

		vk::Fence mRenderFence;
		vk::Semaphore mSwapChainAcquireSemaphore;

		FDestroyQueue mDeletionQueue;

	public:
		friend class FVulkanRHI;
	};

} // Turbo
