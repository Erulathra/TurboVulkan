#pragma once

#include "TaskScheduler.h"
#include "Graphics/GPUDevice.h"

namespace Turbo
{
	struct FIOThread : enki::IPinnedTask
	{
		virtual void Execute() override;
	};

	struct FIOTask : enki::IPinnedTask
	{
		virtual void Execute() override;
	};

	struct FTextureLoadingRequest
	{

	};

	struct FAsyncLoadingManager
	{
	public:
		void Init(FGPUDevice& gpu);
		void Shutdown(FGPUDevice& gpu);

		void Update();

		void RequestTextureLoading(const FTextureLoadingRequest& textureLoadingRequest);

	private:
		vk::CommandPool mVkCommandPool;
		TUniquePtr<FCommandBuffer> mCommandBuffer;
		vk::Semaphore mVkSemaphore;
		vk::Fence mVkFence;

		std::vector<FTextureLoadingRequest> mTextureRequestStack;
		std::mutex mTextureRequestStackCS;
	};
}
