#include <ranges>

#include "Core/DataStructures/DestoryQueue.h"

namespace Turbo
{
	void FDestroyQueue::Flush(void* context)
	{
		for (std::unique_ptr<IDestroyQueue>& destroyQueue : mDestroyQueues | std::views::values)
		{
			destroyQueue->Flush(context);
		}

		mOnDestroy.Broadcast();
		mOnDestroy.RemoveAll();
	}
} // Turbo
