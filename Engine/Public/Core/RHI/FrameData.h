#pragma once

#include "RHICore.h"

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

	private:
		FVulkanDevice* mDevice;

		vk::Fence mRenderFence;

		vk::Semaphore mSwapChainSemaphore;
		vk::Semaphore mRenderSemaphore;

		vk::CommandBuffer mCommandBuffer;

	public:
		friend class FVulkanRHI;
	};

} // Turbo
