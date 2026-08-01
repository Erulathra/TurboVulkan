#pragma once

#include <cstring>
#if PLATFORM_WINDOWS
#include <corecrt_malloc.h>
#endif // PLATFORM_WINDOWS

#include "stdlib.h"

namespace Turbo
{
	struct FArenaAllocator
	{
	public:
		explicit FArenaAllocator(size_t size)
		{
			TRACE_ZONE_SCOPED()

#if PLATFORM_WINDOWS
			mAllocation = static_cast<byte*>(_aligned_malloc(size, 4));
#else // PLATFORM_WINDOWS
			mAllocation = static_cast<byte*>(aligned_alloc(4, size));
#endif // else PLATFORM_WINDOWS
			mTip = mAllocation + size;
			mTop = mAllocation;
		}

		~FArenaAllocator()
		{
#if PLATFORM_WINDOWS
			_aligned_free(mAllocation);
#else // PLATFORM_WINDOWS
			delete[] mAllocation;
#endif // else PLATFORM_WINDOWS
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

		template <typename Type>
		Type* AllocateZeroed()
		{
			return reinterpret_cast<Type*>(AllocateZeroed(sizeof(Type), alignof(Type)));
		}

		template <typename Type>
		Type* AllocateZeroed(size_t num)
		{
			return reinterpret_cast<Type*>(AllocateZeroed(num * sizeof(Type), alignof(Type)));
		}

		byte* AllocateZeroed(size_t size, size_t alignment = 4)
		{
		   byte* result = Allocate(size, alignment);
			std::memset(result, 0, size);

			return result;
		}

		template <typename Type>
		Type* AllocateDefaulted()
		{
			Type* result = reinterpret_cast<Type*>(Allocate(sizeof(Type), alignof(Type)));
			*result = Type{};

			return result;
		}

		template <typename Type>
		Type* AllocateDefaulted(size_t num)
		{
			Type* result = reinterpret_cast<Type*>(Allocate(num * sizeof(Type), alignof(Type)));

			for (uint32 i = 0; i < num; ++num)
			{
            result[i] = Type{};
			}

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
