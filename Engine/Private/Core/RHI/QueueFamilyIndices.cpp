#include "Core/RHI/QueueFamilyIndices.h"

namespace Turbo {
	std::set<uint32> QueueFamilyIndices::GetUniqueQueueIndices() const
	{
		// Return value optimization
		std::set<uint32> Result;

		if (IsValid())
		{
			Result = {
				static_cast<uint32>(GraphicsFamily),
				static_cast<uint32>(PresentFamily)
			};
		}

		return Result;
	}

	bool QueueFamilyIndices::IsValid() const
	{
		return GraphicsFamily != INDEX_NONE
			&& PresentFamily != INDEX_NONE;
	}
} // Turbo