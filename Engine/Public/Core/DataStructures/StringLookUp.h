#pragma once

#include <functional>
#include <string>
#include <string_view>

namespace Turbo
{
	struct FStringHash {
	  using is_transparent = void;

	  [[nodiscard]] size_t operator()(const char* string) const {
		return std::hash<std::string_view>{}(string);
	  }
	  [[nodiscard]] size_t operator()(const std::string_view string) const {
		return std::hash<std::string_view>{}(string);
	  }
	  [[nodiscard]] size_t operator()(const std::string& string) const {
		return std::hash<std::string>{}(string);
	  }
	};

	template<typename ValueType>
	using TStringLookUp = std::unordered_map<std::string, ValueType, FStringHash, std::equal_to<>>;
}
