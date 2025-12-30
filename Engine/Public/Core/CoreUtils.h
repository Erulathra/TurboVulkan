#pragma once

namespace Turbo
{
	namespace CoreUtils
	{
		template<typename T>
		constexpr size_t SizeofOrZero()
		{
			size_t size = 0;
			if constexpr (std::is_same_v<T, void> == false)
			{
				size = sizeof(T);
			}

			return size;
		}
	};
} // Turbo
