#pragma once
#include "CommonMacros.h"
#include "CommonTypeDefs.h"

namespace Turbo
{
	struct FStackAllocator
	{
	public:
		explicit FStackAllocator(size_t stackSize)
		{
			TRACE_ZONE_SCOPED()

			mAllocation = new byte[stackSize];
			mTip = mAllocation + stackSize;
			mTop = mAllocation;
		}

		~FStackAllocator()
		{
			delete[] mAllocation;
		}

		template <typename Type>
		Type* Allocate()
		{
			return reinterpret_cast<Type*>(Allocate(sizeof(Type)));
		}

		byte* Allocate(size_t size)
		{
			TURBO_CHECK(mAllocation != nullptr && mTop != nullptr && mTip != nullptr)
			TURBO_CHECK_MSG(mTop + size < mTip, "Stack allocator overflow")

			byte* result = mTop;
			mTop += size;

			return result;
		}

		void Clear()
		{
			mTop = mAllocation;
		}

	private:
		byte* mAllocation = nullptr;
		byte* mTop = nullptr;
		byte* mTip = nullptr;
	};
}
