#pragma once

namespace Turbo
{
	template<typename T, uint32 size>
	class TArrayHeap final
	{
		using ArrayType = std::array<T, size>;

	public:
		explicit TArrayHeap() { mArrayPtr = std::make_unique<ArrayType>(); }
		ArrayType* operator->() { return mArrayPtr.get(); }
		const ArrayType* operator->() const { return mArrayPtr.get(); }

		T& operator[](uint32 index) { return mArrayPtr->at(index); }
		const T& operator[](uint32 index) const { return mArrayPtr->at(index); }

	private:
		TUniquePtr<ArrayType> mArrayPtr;
	};
} // Turbo