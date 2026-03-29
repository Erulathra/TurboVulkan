#pragma once

#include <string>
#include "fmt/format.h"


namespace ImGui
{
	template <typename... T>
	IMGUI_API void TextFmt(fmt::format_string<T...> fmt, T&&... args)
	{
		const std::string str = fmt::vformat(fmt.str, fmt::vargs<T...>{{args...}});
		TextUnformatted(&*str.begin(), &*str.end());
	}
}
