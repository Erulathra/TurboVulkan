#include <ranges>

#include "Core/RHI/DeletionQueue.h"

namespace Turbo
{
	void FDeletionQueue::Flush()
	{
		for (const std::shared_ptr<IDeletable>& objectToDelete : std::ranges::reverse_view(mObjectsToDelete))
		{
			objectToDelete->Delete();
		}
		mObjectsToDelete.clear();

		for (const FDeletionDelegate& delegateToCall : std::ranges::reverse_view(mDelegatesToCall))
		{
			std::invoke(delegateToCall);
		}
		mDelegatesToCall.clear();
	}
} // Turbo
