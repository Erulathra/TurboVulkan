#pragma once

#define DECLARE_RESOURCE_HANDLE(RESOURCE_NAME) struct F##RESOURCE_NAME##Handle final : public FTypeSafeResourceHandleBase { }

namespace Turbo
{
	using FPoolIndexType = uint32;
	using FResourceHandle = FPoolIndexType;
	inline constexpr FResourceHandle kInvalidHandle = std::numeric_limits<uint32>::max();

	struct FTypeSafeResourceHandleBase
	{
		FResourceHandle Index = kInvalidHandle;
		[[nodiscard]] bool IsValid() const { return Index != kInvalidHandle; }
		void Reset() { Index = kInvalidHandle; }
	};

	template<typename T, typename THandle , uint32 size>
	requires std::is_base_of_v<FTypeSafeResourceHandleBase, THandle>
		  && (size < kInvalidHandle)
	class TResourcePool
	{
	public:
		TResourcePool() { Clear(); }

	public:
		void Clear()
		{
			mFreeIndicesHead = 0;
			mUsedIndices = 0;

			for (FPoolIndexType i = 0; i < size; ++i)
			{
				mFreeIndices[i] = i;
			}
		}

		THandle Acquire()
		{
			TURBO_CHECK_MSG(mFreeIndicesHead < size, "No more resources left!")

			const THandle NewHandle {};
			NewHandle.Index = mFreeIndices[mFreeIndicesHead];

			++mFreeIndicesHead;
			++mUsedIndices;

			return NewHandle;
		}

		void Release(THandle handle)
		{
			--mFreeIndicesHead;
			--mUsedIndices;

			mFreeIndices[mFreeIndicesHead] = handle.Index;
		}

		T* Access(THandle handle)
		{
			if (handle.IsValid())
			{
				return mData[handle.Index];
			}

			return nullptr;
		}

		const T* Access(THandle handle) const
		{
			if (handle.IsValid())
			{
				return mData[handle.Index];
			}

			return nullptr;
		}

	private:
		std::array<T, size> mData;
		std::array<FPoolIndexType, size> mFreeIndices;

		FPoolIndexType mFreeIndicesHead = 0;
		FPoolIndexType mUsedIndices = 0;
	};
} // turbo
