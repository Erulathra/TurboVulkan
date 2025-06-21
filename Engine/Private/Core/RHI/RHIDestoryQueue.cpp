#include <ranges>

#include "Core/RHI/RHIDestoryQueue.h"

#include "Core/RHI/VulkanDevice.h"

namespace Turbo
{
	void FRHIDestroyQueue::Flush(const FVulkanDevice* device)
	{
		TURBO_CHECK(device)

		for (std::unique_ptr<IDestroyQueue>& destroyQueue : mDestroyQueues | std::views::values)
		{
			destroyQueue->Flush(device);
		}

		mOnDestroy.Broadcast();
		mOnDestroy.RemoveAll();
	}
} // Turbo
