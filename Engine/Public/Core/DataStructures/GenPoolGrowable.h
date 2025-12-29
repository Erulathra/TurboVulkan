#pragma once

#include "Core/DataStructures/Handle.h"

namespace Turbo
{
	template<typename T>
	class TGenPoolGrowable final
	{
		static constexpr FHandle::IndexType kInitialSize = 32;

	public:
		TGenPoolGrowable(FHandle::IndexType initialSize = kInitialSize)
		{
			Clear(initialSize);
		}

		void Clear(FHandle::IndexType numElements)
		{
			mData.clear();
			mGenerations.clear();
			mFreeIndices.clear();

			Reserve(numElements);
		}

		void Reserve(FHandle::IndexType numElements)
		{
			if (numElements < mSize)
			{
				return;
			}

			const FHandle::IndexType oldSize = mSize;
			mSize = numElements;

			mData.resize(mSize);
			mGenerations.resize(mSize);
			mFreeIndices.reserve(mSize);


			for (FHandle::IndexType Index = mSize; Index > oldSize; --Index)
			{
				mFreeIndices.push_back(Index - 1);
			}
		}

		THandle<T> Acquire()
		{
			if (mFreeIndices.empty())
			{
				Reserve(mSize == 0 ? kInitialSize : mSize * 2);
			}

			FHandle::IndexType newIndex = mFreeIndices.back();
			mFreeIndices.pop_back();

			THandle<T> newHandle {};
			newHandle.mIndexAndGen = FHandle::CreateIndex(newIndex, mGenerations[newIndex]);

			TURBO_CHECK_MSG(newHandle.GetGeneration() < FHandle::kGenerationMask, "No more resource generations left!")

			return newHandle;
		}

		void Release(THandle<T> handle)
		{
			TURBO_CHECK(mFreeIndices.size() > 0)

			mFreeIndices.push_back(handle.GetIndex());
			++mGenerations[handle.GetIndex()];
		}

		T* Access(THandle<T> handle)
		{
			TRACE_ZONE_SCOPED_N("Access Resource by handle")

			const FHandle::GenerationType currentGeneration = mGenerations[handle.GetIndex()];
			if (handle.IsValid() && handle.GetIndex() < mData.size() && handle.GetGeneration() == currentGeneration)
			{
				return &mData[handle.GetIndex()];
			}

			return nullptr;
		}

		const T* Access(THandle<T> handle) const
		{
			TRACE_ZONE_SCOPED_N("Access Resource by handle")

			const FHandle::GenerationType currentGeneration = mGenerations[handle.GetIndex()];
			if (handle.IsValid() && handle.GetIndex() < mData.size() && handle.GetGeneration() == currentGeneration)
			{
				return &mData[handle.GetIndex()];
			}

			return nullptr;
		}

	private:
		std::vector<T> mData;
		std::vector<FHandle::GenerationType> mGenerations;
		std::vector<FHandle::IndexType> mFreeIndices;

		FHandle::IndexType mSize = 0;
	};
}