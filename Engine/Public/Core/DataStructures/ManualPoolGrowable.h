#pragma once

#include <vector>

#include "CommonMacros.h"
#include "Core/DataStructures/Handle.h"

namespace Turbo
{
	template <typename T>
	class TManualPoolGrowable final
	{
	public:
		explicit TManualPoolGrowable(FHandle::IndexType initialSize = 32)
		{
			Reserve(initialSize);
		}

		void Reserve(FHandle::IndexType numElements)
		{
			const uint32 targetSize = glm::max(numElements, static_cast<FHandle::IndexType>(mData.size() * 2));
			mData.resize(targetSize);
		}

		T& Access(FHandle atIndex)
		{
			const uint32 handleIndex = atIndex.GetIndex();
			TURBO_CHECK(handleIndex)

			if (mData.size() <= handleIndex)
			{
				Reserve(handleIndex + 1);
			}

			return mData[handleIndex];
		}

	private:
		std::vector<T> mData;
	};
}
