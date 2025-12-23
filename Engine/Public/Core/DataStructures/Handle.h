#pragma once

namespace Turbo
{
	struct FHandle
	{
		// Aliases
		using IndexType = uint32;
		using GenerationType = uint16;

		// Constants
		static constexpr IndexType kIndexMask = 0xFFFFF;
		static constexpr IndexType kGenerationMask = 0xFFF;

		static constexpr IndexType kMaxIndex = kIndexMask;
		static constexpr IndexType kMaxGeneration = kGenerationMask;

		static constexpr uint8 kIndexMaskLength = std::popcount(kIndexMask);
		static constexpr IndexType kInvalidIndex = std::numeric_limits<IndexType>::max();

		// Just to make sure that index masks are coherent
		static_assert((kIndexMask | kGenerationMask << kIndexMaskLength) == kInvalidIndex);

		// Statics
		static constexpr IndexType CreateIndex(IndexType index, GenerationType generation)
		{
			return index | (static_cast<IndexType>(generation) << kIndexMaskLength);
		}

		// Helpers
		IndexType GetIndex() const { return mIndexAndGen & kIndexMask; }
		GenerationType GetGeneration() const { return mIndexAndGen & (mIndexAndGen << kIndexMaskLength); }

		[[nodiscard]] constexpr bool IsValid() const { return mIndexAndGen != kInvalidIndex; }
		constexpr void Reset() { mIndexAndGen = kInvalidIndex; }

		explicit constexpr operator bool() const { return IsValid(); }
		constexpr bool operator!() const { return !IsValid(); }

		constexpr bool operator==(const FHandle& rhs) const
		{
			return mIndexAndGen == rhs.mIndexAndGen;
		}

		// Index and Generation
		IndexType mIndexAndGen = kInvalidIndex;
	};

	template <typename ObjectType>
	struct THandle final : public FHandle
	{
	};
}

inline bool operator==(const Turbo::FHandle lhs, const Turbo::FHandle rhs)
{
	return lhs.mIndexAndGen == rhs.mIndexAndGen;
}

template<>
struct std::hash<Turbo::FHandle>
{
	size_t operator()(Turbo::FHandle handle) const noexcept
	{
		return static_cast<size_t>(handle.mIndexAndGen);
	}
};

template<typename T>
inline bool operator==(const Turbo::THandle<T> lhs, const Turbo::THandle<T> rhs)
{
	return lhs.mIndexAndGen == rhs.mIndexAndGen;
}

template<typename T>
struct std::hash<Turbo::THandle<T>>
{
	size_t operator()(Turbo::THandle<T> handle) const noexcept
	{
		return handle.mIndexAndGen;
	}
};
