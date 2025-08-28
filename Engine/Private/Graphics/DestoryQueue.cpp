#include <ranges>

#include "Graphics/DestoryQueue.h"

namespace Turbo
{
	void FDestroyQueue::Flush(FGPUDevice& GPUDevice)
	{
		for (std::unique_ptr<IDestroyQueue>& destroyQueue : mDestroyQueues | std::views::values)
		{
			destroyQueue->Flush(GPUDevice);
		}

		mOnDestroy.Broadcast();
		mOnDestroy.RemoveAll();
	}
} // Turbo
