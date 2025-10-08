#pragma once

namespace Turbo
{
	using FPoolIndexType = uint32;
	inline constexpr FPoolIndexType kInvalidHandle = std::numeric_limits<uint32>::max();

	using FPoolGenerationType = uint32;
	inline constexpr FPoolGenerationType kInvalidGeneration = std::numeric_limits<uint32>::max();

	struct FHandle
	{
		FPoolIndexType mIndex = kInvalidHandle;
		FPoolGenerationType mGeneration = kInvalidGeneration;
	};

	template <typename ObjectType>
	struct THandle final : public FHandle
	{
	public:
		[[nodiscard]] bool IsValid() const { return mIndex != kInvalidHandle; }
		void Reset() { mIndex = kInvalidHandle; }

		explicit operator bool() const { return IsValid(); }
		bool operator!() const { return !IsValid(); }
	};

	template<typename T, uint32 size>
		requires (size < kInvalidHandle)
	class TResourcePool
	{
	public:
		TResourcePool() { Clear(); }

	public:
		[[nodiscard]] static constexpr FPoolIndexType GetCapacity() { return size; }
		[[nodiscard]] FPoolIndexType GetNumAcquiredResources() const { return mUsedIndices; }

		void Clear()
		{
			mFreeIndicesHead = 0;
			mUsedIndices = 0;

			for (FPoolIndexType i = 0; i < size; ++i)
			{
				mFreeIndices[i] = i;
			}

			for (FPoolIndexType i = 0; i < size; ++i)
			{
				mGenerations[i] = 0;
			}
		}

		THandle<T> Acquire()
		{
			TURBO_CHECK_MSG(mFreeIndicesHead < size, "No more resources left!")

			THandle<T> NewHandle {};
			NewHandle.mIndex = mFreeIndices[mFreeIndicesHead];
			NewHandle.mGeneration = mGenerations[NewHandle.mIndex];
			TURBO_CHECK_MSG(NewHandle.mGeneration < kInvalidGeneration, "No more resource generations left!")

			++mFreeIndicesHead;
			++mUsedIndices;

			return NewHandle;
		}

		void Release(THandle<T> handle)
		{
			TURBO_CHECK(mUsedIndices > 0)

			--mFreeIndicesHead;
			--mUsedIndices;

			mFreeIndices[mFreeIndicesHead] = handle.mIndex;
			++mGenerations[handle.mIndex];
		}

		T* Access(THandle<T> handle)
		{
			TRACE_ZONE_SCOPED_N("Access Resource by handle")

			const FPoolGenerationType currentGeneration = mGenerations[handle.mIndex];
			if (handle.IsValid() && handle.mIndex < mData.size() && handle.mGeneration == currentGeneration)
			{
				return &mData[handle.mIndex];
			}

			return nullptr;
		}

		const T* Access(THandle<T> handle) const
		{
			if (handle.IsValid() && handle.mIndex < mData.size())
			{
				return &mData[handle.mIndex];
			}

			return nullptr;
		}

	private:

		std::array<T, size> mData;
		std::array<FPoolGenerationType, size> mGenerations;
		std::array<FPoolIndexType, size> mFreeIndices;

		FPoolIndexType mFreeIndicesHead = 0;
		FPoolIndexType mUsedIndices = 0;
	};

	template<typename T, uint32 size>
		requires (size < kInvalidHandle)
	class TResourcePoolHeap
	{
		using TPoolType = TResourcePool<T, size>;
	public:
		TResourcePoolHeap() { mPoolPtr = MakeUnique<TPoolType>(); }
		TPoolType* operator->() { return mPoolPtr.get(); }
		const TPoolType* operator->() const { return mPoolPtr.get(); }

	private:
		TUniquePtr<TPoolType> mPoolPtr;
	};
} // turbo
