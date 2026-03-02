#pragma once

namespace Turbo::Memory
{
	template <typename T>
	requires std::is_integral_v<T> || std::is_pointer_v<T>
	constexpr T Align(T value, uintptr_t alignment)
	{
		return reinterpret_cast<T>((reinterpret_cast<uintptr_t>(value) + alignment - 1) & ~(alignment - 1));
	}
}