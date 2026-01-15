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

	void FAsyncLoadingManager::Update()
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		TURBO_LOG(LogTemp, Info, "IO THREAD WORK...")
	}
}
