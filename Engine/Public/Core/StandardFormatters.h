#pragma once

template <>
struct fmt::formatter<std::filesystem::path> : fmt::formatter<std::string>
{
	auto format(const std::filesystem::path& path, format_context& ctx) const
	{
		return formatter<std::string>::format(path.string(), ctx);
	}
};
