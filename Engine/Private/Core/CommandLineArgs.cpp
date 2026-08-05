#include "Core/CommandLineArgs.h"
#include "Core/Utils/StringUtils.h"
#include <filesystem>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace Turbo
{
   std::filesystem::path FCommandLineArgs::mWorkingDir;
   std::unordered_map<std::string_view, std::string_view> FCommandLineArgs::mArgumentsValues;
   std::unordered_set<std::string_view> FCommandLineArgs::mFlags;
   std::vector<std::string_view> FCommandLineArgs::mLooseArguments;

	bool DoesArgumentStartsWithDashes(std::string_view arg)
	{
		return arg.length() > 2 && arg[0] == '-' && arg[1] == '-';
	}

	void FCommandLineArgs::Parse(const uint32 argc, char* argv[])
	{
		// Working dir is always at 0
		mWorkingDir = std::filesystem::path(argv[0]);

		uint32 argIndex = 1;
		while (argIndex < argc)
		{
			const std::string_view arg = std::string_view(argv[argIndex]);

			if (DoesArgumentStartsWithDashes(arg))
			{
			   const std::string_view argWithoutDashes = std::string_view(arg.begin() + 2, arg.end());

				if (argIndex + 1 < argc)
				{
					const std::string_view nextArg = std::string_view(argv[argIndex + 1]);
					if (DoesArgumentStartsWithDashes(nextArg))
					{
          		   mFlags.emplace(argWithoutDashes);
                  ++argIndex;
					}
					else
					{
          		   mArgumentsValues.try_emplace(argWithoutDashes, nextArg);
          		   argIndex += 2;
					}
				}
				else
				{
   				mFlags.emplace(argWithoutDashes);
               ++argIndex;
				}
			}
			else
			{
				mLooseArguments.push_back(arg);
				++argIndex;
			}
		}
	}

	bool FCommandLineArgs::HasFlag(std::string_view argument)
	{
      return mFlags.find(argument) != mFlags.end() || mArgumentsValues.find(argument) != mArgumentsValues.end();
	}

   std::optional<std::string_view> FCommandLineArgs::ParseString(std::string_view argumentName)
   {
      if (auto foundIt = mArgumentsValues.find(argumentName);
         foundIt != mArgumentsValues.end())
      {
         return foundIt->second;
      }
      else if (auto foundIt = mFlags.find(argumentName);
         foundIt != mFlags.end())
      {
         return "";
      }

      return {};
   }

   std::optional<int32> FCommandLineArgs::ParseInt(std::string_view argumentName)
   {
      if (std::optional<std::string_view> stringValue = ParseString(argumentName);
         stringValue.has_value())
      {
         return StringUtils::ParseInt(stringValue.value());
      }

      return {};
   }

   std::optional<float> FCommandLineArgs::ParseFloat(std::string_view argumentName)
   {
      if (std::optional<std::string_view> stringValue = ParseString(argumentName);
         stringValue.has_value())
      {
         return StringUtils::ParseFloat(stringValue.value());
      }

      return {};
   }

   const std::filesystem::path& FCommandLineArgs::GetWorkingDir()
   {
      return mWorkingDir;
   }

   std::vector<std::string_view>& FCommandLineArgs::GetLooseArgs()
   {
      return mLooseArguments;
   }
}
