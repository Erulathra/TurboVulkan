#pragma once
#include "CommonMacros.h"
#include "CommonTypeDefs.h"
#include "Core/Memory.h"

namespace Turbo
{
	struct FArenaAllocator
	{
	public:
		explicit FArenaAllocator(size_t size)
		{
			TRACE_ZONE_SCOPED()

			mAllocation = static_cast<byte*>(aligned_alloc(4, size));
			mTip = mAllocation + size;
			mTop = mAllocation;
		}

		~FArenaAllocator()
		{
			delete[] mAllocation;
		}

		template <typename Type>
		Type* Allocate()
		{
			return reinterpret_cast<Type*>(Allocate(sizeof(Type), alignof(Type)));
		}

		template <typename Type>
		Type* Allocate(size_t num)
		{
			return reinterpret_cast<Type*>(Allocate(num * sizeof(Type), alignof(Type)));
		}

		byte* Allocate(size_t size, size_t alignment = 4)
		{
			TURBO_CHECK(mAllocation != nullptr && mTop != nullptr && mTip != nullptr)
			TURBO_CHECK((alignment & (alignment-1)) == 0)
			TURBO_CHECK(size > 0)

			// Align new top
			byte* result = Memory::Align(mTop, alignment);
			byte* newTop = result + size;
			TURBO_CHECK_MSG(newTop <= mTip, "Stack allocator overflow")

			mTop = newTop;

			return result;
		}

		bool Contains(void* ptr, size_t size = 0) const
		{
			return ptr >= mAllocation && static_cast<byte*>(ptr) + size <= mTop;
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
