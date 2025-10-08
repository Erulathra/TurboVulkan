#pragma once

namespace Turbo
{
	using FPoolIndexType = uint32;
	using FResourceHandle = FPoolIndexType;
	inline constexpr FResourceHandle kInvalidHandle = std::numeric_limits<uint32>::max();

	template <typename ObjectType>
	struct THandle final
	{
		FResourceHandle Index = kInvalidHandle;

		[[nodiscard]] bool IsValid() const { return Index != kInvalidHandle; }
		void Reset() { Index = kInvalidHandle; }

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
		[[nodiscard]] static constexpr FResourceHandle GetCapacity() { return size; }
		[[nodiscard]] FResourceHandle GetNumAcquiredResources() const { return mUsedIndices; }

		void Clear()
		{
			mFreeIndicesHead = 0;
			mUsedIndices = 0;

			for (FPoolIndexType i = 0; i < size; ++i)
			{
				mFreeIndices[i] = i;
			}
		}

		THandle<T> Acquire()
		{
			TURBO_CHECK_MSG(mFreeIndicesHead < size, "No more resources left!")

			THandle<T> NewHandle {};
			NewHandle.Index = mFreeIndices[mFreeIndicesHead];

			++mFreeIndicesHead;
			++mUsedIndices;

			return NewHandle;
		}

		void Release(THandle<T> handle)
		{
			TURBO_CHECK(mUsedIndices > 0)

			--mFreeIndicesHead;
			--mUsedIndices;

			if constexpr (std::is_default_constructible_v<T>)
			{
				mData[handle.Index] = T();
			}

			mFreeIndices[mFreeIndicesHead] = handle.Index;
		}

		T* Access(THandle<T> handle)
		{
			TRACE_ZONE_SCOPED_N("Access Resource by handle")

			if (handle.IsValid() && handle.Index < mData.size())
			{
				return &mData[handle.Index];
			}

			return nullptr;
		}

		const T* Access(THandle<T> handle) const
		{
			if (handle.IsValid() && handle.Index < mData.size())
			{
				return &mData[handle.Index];
			}

			return nullptr;
		}

	private:
		std::array<T, size> mData;
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
