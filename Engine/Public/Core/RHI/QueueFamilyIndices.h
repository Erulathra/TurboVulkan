#pragma once

namespace Turbo {
	struct FQueueFamilyIndices
	{
		int32 GraphicsFamily = INDEX_NONE;
		int32 PresentFamily = INDEX_NONE;
		int32 TransferFamily = INDEX_NONE;

		[[nodiscard]] bool IsValid() const;
		[[nodiscard]] uint32 Num() const { return 3; }
		[[nodiscard]] std::set<uint32> GetUniqueQueueIndices() const;
	};
} // Turbo
