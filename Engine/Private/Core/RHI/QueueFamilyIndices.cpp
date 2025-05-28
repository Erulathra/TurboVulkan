#include "Core/RHI/QueueFamilyIndices.h"

namespace Turbo {
	std::set<uint32> FQueueFamilyIndices::GetUniqueQueueIndices() const
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

	bool FQueueFamilyIndices::IsValid() const
	{
		return GraphicsFamily != INDEX_NONE
			&& PresentFamily != INDEX_NONE;
	}
} // Turbo