#pragma once

namespace Turbo
{
	struct FEnviromentalVariables
	{
	public:
		static bool GetFlag(std::string_view variableName);
		static std::optional<int32> GetInt(std::string_view variableName);
		static std::optional<std::string> GetString(std::string_view variableName);

		static std::optional<std::string> FindOrGetEnvVariable(std::string_view variableName);

	public:
		static entt::dense_map<std::string, std::string> mVariables;

		static entt::dense_set<std::string> mUndefinedVariables;
	};
} // namespace Turbo
