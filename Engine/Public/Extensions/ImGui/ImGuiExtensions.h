#pragma once

#include <string>
#include "fmt/format.h"


namespace ImGui
{

	template <typename... Args>
	void TextFmt(fmt::format_string<Args...> fmt, Args&&... args)
	{
		const std::string str = fmt::format(fmt, std::forward<Args>(args)...);
		TextUnformatted(&*str.begin(), &*str.end());
	}
}
