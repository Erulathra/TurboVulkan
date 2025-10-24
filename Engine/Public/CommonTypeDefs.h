#pragma once

#include <memory>

// integer types
using int8 = std::int8_t;
using uint8 = std::uint8_t;
using byte = int8;

using int16 = std::int16_t;
using uint16 = std::uint16_t;

using int32 = std::int32_t;
using uint32 = std::uint32_t;

using int64 = std::int64_t;
using uint64 = std::uint64_t;

using cstring = const char*;

namespace Turbo
{
	template<typename T>
	using TSharedPtr = std::shared_ptr<T>;

	template<typename T>
	const auto MakeShared = std::make_shared<T>;

	template<typename T>
	using TWeakPtr = std::weak_ptr<T>;

	template<typename T>
	using TUniquePtr = std::unique_ptr<T>;

	template<typename T>
	const auto MakeUnique = std::make_unique<T>;
}