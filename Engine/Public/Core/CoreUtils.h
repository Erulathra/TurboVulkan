#pragma once

#include <cstddef>
namespace Turbo
{
	namespace CoreUtils
	{
		template <typename T>
		constexpr size_t SizeofOrZero()
		{
			size_t size = 0;
			if constexpr (std::is_same_v<T, void> == false)
			{
				size = sizeof(T);
			}

			return size;
		}

		// Source: https://stackoverflow.com/questions/2590677/how-do-i-combine-hash-values-in-c0x
		template <typename T, typename... Rest>
		void HashCombine(uint32& inOutHash, const T& v, const Rest&... rest)
		{
			inOutHash ^= std::hash<T>{}(v) + 0x9e3779b9u + (inOutHash << 6) + (inOutHash >> 2);
			(HashCombine(inOutHash, rest), ...);
		};

		template <typename T, typename... Rest>
		void HashCombine(std::size_t& inOutHash, const T& v, const Rest&... rest)
		{
			inOutHash ^= std::hash<T>{}(v) + 0x9e3779b97f4a7c13ull + (inOutHash << 12) + (inOutHash >> 4);
			(HashCombine(inOutHash, rest), ...);
		};

		inline uint32 Adler32Hash(std::span<byte> data)
		{
			uint32 a = 1, b = 0;

			for (size_t index = 0; index < data.size(); ++index)
			{
   			constexpr uint32 modulo = 65521;

				a = (a + (uint32)data[index]) % modulo;
				b = (b + a) % modulo;
			}

			return (b << 16) | a;
		}
	} // namespace CoreUtils
} // namespace Turbo
