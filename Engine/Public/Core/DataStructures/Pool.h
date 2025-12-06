#pragma once
#include <stack>

namespace Turbo
{
	using FPoolIndexType = uint16;
	inline constexpr FPoolIndexType kInvalidHandle = std::numeric_limits<FPoolIndexType>::max();

	using FPoolGenerationType = uint16;
	inline constexpr FPoolGenerationType kInvalidGeneration = std::numeric_limits<FPoolGenerationType>::max();

	struct FHandle
	{
		FPoolIndexType mIndex = kInvalidHandle;
		FPoolGenerationType mGeneration = kInvalidGeneration;

		[[nodiscard]] constexpr bool IsValid() const { return mIndex != kInvalidHandle && mGeneration != kInvalidGeneration; }
		constexpr void Reset() { mIndex = kInvalidHandle; mGeneration = kInvalidGeneration; }

		explicit constexpr operator bool() const { return IsValid(); }
		constexpr bool operator!() const { return !IsValid(); }

		constexpr bool operator==(const FHandle& rhs) const
		{
			return mIndex == rhs.mIndex
				&& mGeneration == rhs.mGeneration;
		}
	};

	template <typename ObjectType>
	struct THandle final : public FHandle
	{
	};

	template<typename T, FPoolIndexType size>
		requires (size < kInvalidHandle)
	class TPool
	{
	public:
		TPool() { Clear(); }

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

			if (handle.IsValid() && handle.mIndex < mData.size())
			{
				if (handle.mGeneration == mGenerations[handle.mIndex])
				{
					return &mData[handle.mIndex];
				}
			}

			return nullptr;
		}

		const T* Access(THandle<T> handle) const
		{
			TRACE_ZONE_SCOPED_N("Access Resource by handle")

			if (handle.IsValid() && handle.mIndex < mData.size())
			{
				if (handle.mGeneration == mGenerations[handle.mIndex])
				{
					return &mData[handle.mIndex];
				}
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

	template<typename T, FPoolIndexType size>
		requires (size < kInvalidHandle)
	class TPoolHeap
	{
		using TPoolType = TPool<T, size>;
	public:
		TPoolHeap() { mPoolPtr = MakeUnique<TPoolType>(); }
		TPoolType* operator->() { return mPoolPtr.get(); }
		const TPoolType* operator->() const { return mPoolPtr.get(); }

	private:
		TUniquePtr<TPoolType> mPoolPtr;
	};

	template<typename T>
	class TPoolGrowable final
	{
		static constexpr FPoolIndexType kInitialSize = 32;

	public:
		TPoolGrowable(FPoolIndexType initialSize = kInitialSize)
		{
			Clear(initialSize);
		}

		void Clear(FPoolIndexType numElements)
		{
			mData.clear();
			mGenerations.clear();
			mFreeIndices.clear();

			Reserve(numElements);
		}

		void Reserve(FPoolIndexType numElements)
		{
			if (numElements < mSize)
			{
				return;
			}

			const FPoolIndexType oldSize = mSize;
			mSize = numElements;

			mData.resize(mSize);
			mGenerations.resize(mSize);
			mFreeIndices.reserve(mSize);


			for (FPoolIndexType Index = mSize; Index > oldSize; --Index)
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

			THandle<T> NewHandle {};
			NewHandle.mIndex = mFreeIndices.back();
			mFreeIndices.pop_back();
			NewHandle.mGeneration = mGenerations[NewHandle.mIndex];
			TURBO_CHECK_MSG(NewHandle.mGeneration < kInvalidGeneration, "No more resource generations left!")

			return NewHandle;
		}

		void Release(THandle<T> handle)
		{
			TURBO_CHECK(mFreeIndices.size() > 0)

			mFreeIndices.push_back(handle.mIndex);
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
			TRACE_ZONE_SCOPED_N("Access Resource by handle")

			const FPoolGenerationType currentGeneration = mGenerations[handle.mIndex];
			if (handle.IsValid() && handle.mIndex < mData.size() && handle.mGeneration == currentGeneration)
			{
				return &mData[handle.mIndex];
			}

			return nullptr;
		}

	private:
		std::vector<T> mData;
		std::vector<FPoolGenerationType> mGenerations;
		std::vector<FPoolIndexType> mFreeIndices;

		FPoolIndexType mSize = 0;
	};
} // turbo

inline bool operator==(const Turbo::FHandle lhs, const Turbo::FHandle rhs)
{
	return lhs.mIndex == rhs.mIndex
		&& lhs.mGeneration == rhs.mGeneration;
}

template<>
struct std::hash<Turbo::FHandle>
{
	std::size_t operator()(Turbo::FHandle handle) const noexcept
	{
		const std::size_t h1 = std::hash<Turbo::FPoolIndexType>{}(handle.mIndex);
		const std::size_t h2 = std::hash<Turbo::FPoolGenerationType>{}(handle.mGeneration);
		return h1 ^ (h2 << 1);
	}
};

template<typename T>
inline bool operator==(const Turbo::THandle<T> lhs, const Turbo::THandle<T> rhs)
{
	return lhs.mIndex == rhs.mIndex
		&& lhs.mGeneration == rhs.mGeneration;
}

template<typename T>
struct std::hash<Turbo::THandle<T>>
{
	std::size_t operator()(Turbo::THandle<T> handle) const noexcept
	{
		const std::size_t h1 = std::hash<Turbo::FPoolIndexType>{}(handle.mIndex);
		const std::size_t h2 = std::hash<Turbo::FPoolGenerationType>{}(handle.mGeneration);
		return h1 ^ (h2 << 1);
	}
};
