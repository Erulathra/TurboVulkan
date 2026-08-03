#include "Core/EnviromentalVariables.h"
#include "Core/Platform.h"
#include "Core/Utils/StringUtils.h"
#include "entt/container/dense_set.hpp"
#include <optional>

namespace Turbo
{
   entt::dense_map<std::string, std::string> FEnviromentalVariables::mVariables;
   entt::dense_set<std::string> FEnviromentalVariables::mUndefinedVariables;

	bool FEnviromentalVariables::GetFlag(std::string_view variableName)
	{
      return FindOrGetEnvVariable(variableName).has_value();
	}

	std::optional<int32> FEnviromentalVariables::GetInt(std::string_view variableName)
	{
	   if (const std::optional<std::string> foundVariable = FindOrGetEnvVariable(variableName);
			foundVariable.has_value())
		{
		   return StringUtils::ParseInt(foundVariable.value());
		}

      return {};
	}

	std::optional<std::string> FEnviromentalVariables::GetString(std::string_view variableName)
	{
	   return FindOrGetEnvVariable(variableName);
	}

	std::optional<std::string> FEnviromentalVariables::FindOrGetEnvVariable(std::string_view variableName)
	{
		const std::string variableNameString{variableName};

		if (auto foundIt = mVariables.find(variableNameString);
         foundIt != mVariables.end())
		{
			return foundIt->second;
		}
		else if (auto foundIt = mUndefinedVariables.find(variableNameString);
   	   foundIt != mUndefinedVariables.end())
		{
			return {};
		}
		else if (std::optional<std::string> rawOptionalValue = FPlatform::GetEnviromentalVariable(variableName);
			rawOptionalValue.has_value())
		{
			mVariables.emplace(variableNameString, rawOptionalValue.value());
			return rawOptionalValue;
		}

		mUndefinedVariables.emplace(variableName);
		return {};
	}
} // namespace Turbo
