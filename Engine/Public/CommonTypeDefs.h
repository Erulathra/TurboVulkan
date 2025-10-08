#pragma once

#include <cstdint>
#include <memory>

// integer types
using int8 = int8_t;
using uint8 = uint8_t;
using byte = uint8;

using int16 = int16_t;
using uint16 = uint16_t;

using int32 = int32_t;
using uint32 = uint32_t;

using int64 = int64_t;
using uint64 = uint64_t;

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