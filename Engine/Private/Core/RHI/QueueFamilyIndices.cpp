#include "Core/RHI/QueueFamilyIndices.h"

namespace Turbo {
	std::set<uint32> FQueueFamilyIndices::GetUniqueQueueIndices() const
	{
		// Return value optimization
		std::set<uint32> result;

		if (IsValid())
		{
			result = {
				static_cast<uint32>(GraphicsFamily),
				static_cast<uint32>(PresentFamily),
				static_cast<uint32>(TransferFamily)
			};
		}

		return result;
	}

	bool FQueueFamilyIndices::IsValid() const
	{
		return GraphicsFamily != INDEX_NONE
			&& PresentFamily != INDEX_NONE
			&& TransferFamily != INDEX_NONE;
	}
} // Turbo