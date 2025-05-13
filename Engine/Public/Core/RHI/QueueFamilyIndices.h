#pragma once

namespace Turbo {
	struct QueueFamilyIndices
	{
		int32 GraphicsFamily = INDEX_NONE;
		int32 PresentFamily = INDEX_NONE;

		[[nodiscard]] bool IsValid() const;
		[[nodiscard]] uint32 Num() const { return 2; }
		[[nodiscard]] std::set<uint32> GetUniqueQueueIndices() const;
	};
} // Turbo
