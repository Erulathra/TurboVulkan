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
			mData.resize(numElements);
		}

		T& Access(FHandle atIndex)
		{
			const uint32 handleIndex = atIndex.GetIndex();
			TURBO_CHECK(handleIndex)

			if (mData.size() < handleIndex)
			{
				Reserve(handleIndex);
			}

			return mData[handleIndex];
		}

	private:
		std::vector<T> mData;
	};
}
