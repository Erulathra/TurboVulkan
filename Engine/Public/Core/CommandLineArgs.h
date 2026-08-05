#pragma once

#include <string_view>
#include <unordered_set>
#include <vector>
namespace Turbo
{
	struct FCommandLineArgs
	{
	public:
		static void Parse(uint32 argc, char* argv[]);

		static bool HasFlag(std::string_view argument);
		static std::optional<std::string_view> ParseString(std::string_view argumentName);
		static std::optional<int32> ParseInt(std::string_view argumentName);
		static std::optional<float> ParseFloat(std::string_view argumentName);
		static const std::filesystem::path& GetWorkingDir();

		static std::vector<std::string_view>& GetLooseArgs();

	public:
	   static std::filesystem::path mWorkingDir;
	   static std::unordered_map<std::string_view, std::string_view> mArgumentsValues;
	   static std::unordered_set<std::string_view> mFlags;
	   static std::vector<std::string_view> mLooseArguments;
	};
} // namespace Turbo
