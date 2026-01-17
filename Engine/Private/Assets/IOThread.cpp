#include "Assets/IOThread.h"

namespace Turbo
{
	void FIOThread::Execute()
	{
		enki::TaskScheduler& taskScheduler = entt::locator<enki::TaskScheduler>::value();
		while (taskScheduler.GetIsShutdownRequested() == false)
		{
			taskScheduler.WaitForNewPinnedTasks();
			taskScheduler.RunPinnedTasks();
		}
	}

	void FIOTask::Execute()
	{
		const enki::TaskScheduler& taskScheduler = entt::locator<enki::TaskScheduler>::value();
		FAsyncLoadingManager& asyncLoadingManager = entt::locator<FAsyncLoadingManager>::value();

		while (taskScheduler.GetIsShutdownRequested() == false)
		{
			asyncLoadingManager.Update();
		}
	}

	void FAsyncLoadingManager::Init(FGPUDevice& gpu)
	{
#if 0
		mVkCommandPool = gpu.CreateCommandPool(gpu.GetTransferQueueFamily(), vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
		mCommandBuffer = gpu.CreateCommandBuffer(mVkCommandPool, FName("IOCommandBuffer"));
#endif
	}

	void FAsyncLoadingManager::Shutdown(FGPUDevice& gpu)
	{
	}

	void FAsyncLoadingManager::Update()
	{
	}

	void FAsyncLoadingManager::RequestTextureLoading(const FTextureLoadingRequest& textureLoadingRequest)
	{
		std::scoped_lock scopedLock(mTextureRequestStackCS);
		mTextureRequestStack.push_back(textureLoadingRequest);
	}
}
