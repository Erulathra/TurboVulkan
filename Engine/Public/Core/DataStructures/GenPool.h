#pragma once

#include "Core/DataStructures/Handle.h"

namespace Turbo
{
	template<typename HotType, typename ColdType, FHandle::IndexType size>
		requires (size < FHandle::kMaxIndex)
	class TGenPool
	{
	public:
		TGenPool() { Clear(); }

	public:
		[[nodiscard]] static constexpr FHandle::IndexType GetCapacity() { return size; }
		[[nodiscard]] FHandle::IndexType GetNumAcquiredResources() const { return mUsedIndices; }

		void Clear()
		{
			mFreeIndicesHead = 0;
			mUsedIndices = 0;

			for (FHandle::IndexType i = 0; i < size; ++i)
			{
				mFreeIndices[i] = i;
			}

			for (FHandle::IndexType i = 0; i < size; ++i)
			{
				mGenerations[i] = 0;
			}
		}

		THandle<HotType> Acquire()
		{
			TURBO_CHECK_MSG(mFreeIndicesHead < size, "No more resources left!")

			FHandle::IndexType newIndex = mFreeIndices[mFreeIndicesHead];

			THandle<HotType> NewHandle {};
			NewHandle.mIndexAndGen = FHandle::CreateIndex(newIndex, mGenerations[newIndex]);
			TURBO_CHECK_MSG(NewHandle.GetGeneration() < FHandle::kMaxGeneration, "No more resource generations left!")

			++mFreeIndicesHead;
			++mUsedIndices;

			return NewHandle;
		}

		void Release(THandle<HotType> handle)
		{
			TURBO_CHECK(mUsedIndices > 0)

			--mFreeIndicesHead;
			--mUsedIndices;

			mFreeIndices[mFreeIndicesHead] = handle.GetIndex();
			++mGenerations[handle.GetIndex()];
		}

		HotType* Access(THandle<HotType> handle)
		{
			TRACE_ZONE_SCOPED_N("Access Resource by handle")

			if (handle.IsValid() && handle.GetIndex() < mHotData.size())
			{
				if (handle.GetGeneration() == mGenerations[handle.GetIndex()])
				{
					return &mHotData[handle.GetIndex()];
				}
			}

			return nullptr;
		}

		const HotType* Access(THandle<HotType> handle) const
		{
			TRACE_ZONE_SCOPED_N("Access Resource by handle")

			if (handle.IsValid() && handle.GetIndex() < mHotData.size())
			{
				if (handle.GetGeneration() == mGenerations[handle.GetIndex()])
				{
					return &mHotData[handle.GetIndex()];
				}
			}

			return nullptr;
		}

		ColdType* AccessCold(THandle<HotType> handle)
		{
			TRACE_ZONE_SCOPED_N("Access Resource by handle")

			if (handle.IsValid() && handle.GetIndex() < mHotData.size())
			{
				if (handle.GetGeneration() == mGenerations[handle.GetIndex()])
				{
					return &mColdData[handle.GetIndex()];
				}
			}

			return nullptr;
		}

		const HotType* AccessCold(THandle<HotType> handle) const
		{
			TRACE_ZONE_SCOPED_N("Access Resource by handle")

			if (handle.IsValid() && handle.GetIndex() < mHotData.size())
			{
				if (handle.GetGeneration() == mGenerations[handle.GetIndex()])
				{
					return &mColdData[handle.GetIndex()];
				}
			}

			return nullptr;
		}

	private:
		std::array<HotType, size> mHotData;
		std::array<ColdType, size> mColdData;
		std::array<FHandle::GenerationType, size> mGenerations;
		std::array<FHandle::IndexType, size> mFreeIndices;

		FHandle::IndexType mFreeIndicesHead = 0;
		FHandle::IndexType mUsedIndices = 0;
	};

	template<typename HotType, typename ColdType, FHandle::IndexType size>
		requires (size < FHandle::kMaxIndex)
	class TPoolHeap
	{
		using TPoolType = TGenPool<HotType, ColdType, size>;
	public:
		TPoolHeap() { mPoolPtr = MakeUnique<TPoolType>(); }
		TPoolType* operator->() { return mPoolPtr.get(); }
		const TPoolType* operator->() const { return mPoolPtr.get(); }

	private:
		TUniquePtr<TPoolType> mPoolPtr;
	};

} // turbo

