#include "Debug/IConsoleManager.h"

#include "Core/Engine.h"
#include "Core/Utils/StringUtils.h"

namespace Turbo
{
	namespace BuiltInCommands
	{
		static FAutoConsoleCommand gConsoleCommand_Echo(
			"echo",
			"Prints a message to the console.",
			FConsoleCommandDelegate::CreateLambda([](IConsoleManager& consoleManager, const FArgsVector args)
			{
				uint32 messageLength = 0;
				for (const std::string_view& arg : args)
				{
					messageLength += arg.size();
				}

				std::string message;
				message.reserve(messageLength);

				for (const std::string_view& arg : args)
				{
					message += arg;
					message += " ";
				}

				consoleManager.Print(message);
			}));

		static FAutoConsoleCommand gConsoleCommand_Version(
			"version",
			"Shows build information",
			FConsoleCommandDelegate::CreateLambda([](IConsoleManager& consoleManager, const FArgsVector args)
			{
				std::string message;
				message += "Turbo Engine \n";
				message += fmt::format("Version: {}.{}.{}", TURBO_VERSION_MAJOR, TURBO_VERSION_MINOR, TURBO_VERSION_PATCH);
#if TURBO_BUILD_DEVELOPMENT
				message += "_DEVELOPMENT";
#elif TURBO_BUILD_TEST
				message += "_TEST";
#endif

				consoleManager.Print(message);
			}));

		static FAutoConsoleCommand gConsoleCommand_Exit(
			"exit",
			"Shut downs the engine",
			FConsoleCommandDelegate::CreateLambda([](IConsoleManager& consoleManager, const FArgsVector args)
			{
				gEngine->RequestExit(EExitCode::Success);
			}));
	}

	FAutoConsoleCommand::FAutoConsoleCommand(std::string_view name, std::string_view description, FConsoleCommandDelegate delegate)
	{
		const bool bResult = IConsoleManager::GetSafe().RegisterCommand({
			.mName = std::string(name),
			.mDescription = std::string(description),
			.mDelegate = std::move(delegate)
		});

		TURBO_CHECK_MSG(bResult, "There could be only one command with {} name", name)
	}

	bool FConsoleVariable::Parse(const std::string_view arg) const
	{
		TURBO_CHECK(mDataPtr != nullptr)
		switch (mType)
		{
		case EConsoleVariableType::Bool:
			{
				if (const std::optional<bool> result = StringUtils::ParseBool(arg);
					result.has_value())
				{
					Set(result.value());
					return true;
				}
				break;
			}
		case EConsoleVariableType::Int32:
			{
				if (const std::optional<int32> result = StringUtils::ParseInt(arg);
					result.has_value())
				{
					Set(result.value());
					return true;
				}
				break;
			}
		case EConsoleVariableType::Float:
			{
				if (const std::optional<float> result = StringUtils::ParseFloat(arg);
					result.has_value())
				{
					Set(result.value());
					return true;
				}
				break;
			}
		default:
			TURBO_UNINPLEMENTED()
		}

		return false;
	}

	void FConsoleVariable::Set(bool bValue) const
	{
		TURBO_CHECK(mType == EConsoleVariableType::Bool && mDataPtr != nullptr)
		*static_cast<bool*>(mDataPtr) = bValue;
	}

	void FConsoleVariable::Set(int32 value) const
	{
		TURBO_CHECK(mType == EConsoleVariableType::Int32 && mDataPtr != nullptr)
		*static_cast<int32*>(mDataPtr) = value;
	}

	void FConsoleVariable::Set(float value) const
	{
		TURBO_CHECK(mType == EConsoleVariableType::Float && mDataPtr != nullptr)
		*static_cast<int32*>(mDataPtr) = value;
	}

	bool FConsoleVariable::GetBool() const
	{
		TURBO_CHECK(mType == EConsoleVariableType::Bool && mDataPtr != nullptr)
		return *static_cast<bool*>(mDataPtr);
	}

	int32 FConsoleVariable::GetInt() const
	{
		TURBO_CHECK(mType == EConsoleVariableType::Int32 && mDataPtr != nullptr)
		return *static_cast<int32*>(mDataPtr);
	}

	float FConsoleVariable::GetFloat() const
	{
		TURBO_CHECK(mType == EConsoleVariableType::Float && mDataPtr != nullptr)
		return *static_cast<float*>(mDataPtr);
	}

	std::string FConsoleVariable::ValueToString() const
	{
		std::string result;
		switch (mType) {
		case EConsoleVariableType::Bool:
			result = GetBool() ? "True" : "False";
			break;
		case EConsoleVariableType::Int32:
			result = std::to_string(GetInt());
			break;
		case EConsoleVariableType::Float:
			result = std::to_string(GetFloat());
			break;
		default:
			TURBO_UNINPLEMENTED()
		}

		return result;
	}

	FConsoleManager& FConsoleManager::GetSafe()
	{
		if (entt::locator<IConsoleManager>::has_value() == false)
		{
			entt::locator<IConsoleManager>::emplace<FConsoleManager>();
		}
		return entt::locator<IConsoleManager>::value();
	}

	FConsoleManager& FConsoleManager::Get()
	{
		return entt::locator<IConsoleManager>::value();
	}

	bool FConsoleManager::RegisterCommand(const FConsoleCommand& consoleCommand)
	{
		auto emplaceResult = mCommands.emplace(consoleCommand.mName, consoleCommand);
		return emplaceResult.second;
	}

	bool FConsoleManager::UnregisterCommand(const FConsoleCommand& consoleCommand)
	{
		return UnregisterCommand(consoleCommand.mName);
	}

	bool FConsoleManager::UnregisterCommand(const std::string_view consoleCommandName)
	{
		if (const auto it = mCommands.find(consoleCommandName);
			it != mCommands.end())
		{
			mCommands.erase(it);
			return true;
		}

		return false;
	}

	bool FConsoleManager::RegisterConsoleVariable(const FConsoleVariable& consoleVariable)
	{
		auto emplaceResult = mVariables.emplace(consoleVariable.mName, consoleVariable);
		return emplaceResult.second;
	}

	bool FConsoleManager::UnregisterConsoleVariable(const FConsoleVariable& consoleVariable)
	{
		return UnregisterConsoleVariable(consoleVariable.mName);
	}

	bool FConsoleManager::UnregisterConsoleVariable(const std::string_view consoleVariableName)
	{
		if (const auto it = mVariables.find(consoleVariableName);
			it != mVariables.end())
		{
			mVariables.erase(it);
			return true;
		}

		return false;
	}

	FConsoleCommand* FConsoleManager::FindConsoleCommand(const std::string_view name)
	{
		if (const auto it = mCommands.find(name); it != mCommands.end())
		{
			return &it->second;
		}

		return nullptr;
	}

	FConsoleVariable* FConsoleManager::FindConsoleVariable(const std::string_view name)
	{
		if (const auto it = mVariables.find(name); it != mVariables.end())
		{
			return &it->second;
		}

		return nullptr;
	}

	void FConsoleManager::Print(const std::string_view message)
	{
		FConsoleBufferChangedEvent event {};
		event.mMessage = message;

		gEngine->PushEvent(event);
	}

	void FConsoleManager::Parse(const std::string_view input)
	{
		const std::vector<std::string_view> commandParts = StringUtils::SplitString(input, ' ');

		if (commandParts.size() < 1)
		{
			return;
		}

		const std::string_view command = commandParts[0];
		const std::span arguments = std::span(commandParts.begin() + 1, commandParts.end());

		if (const FConsoleCommand* foundCommand = FindConsoleCommand(command))
		{
			if (arguments.size() > 0 && arguments[0] == "?")
			{
				Printf(
					"Console command: `{}`\n"
					"{}",
					foundCommand->mName,
					foundCommand->mDescription
				);
			}
			else
			{
				foundCommand->mDelegate.ExecuteIfBound(*this, arguments);
			}

			return;
		}

		if (const FConsoleVariable* foundVariable = FindConsoleVariable(command))
		{
			if (arguments.empty() || (arguments.size() == 1 && arguments[0] == "?"))
			{
				Printf(
					"Console variable: `{}`\n"
					"Value: `{}`\n"
					"{}",
					foundVariable->mName,
					foundVariable->ValueToString(),
					foundVariable->mDescription
				);
			}
			else if (arguments.size() == 1)
			{
				if (foundVariable->Parse(arguments[0]))
				{
					Printf("Set {} to {}", foundVariable->mName, foundVariable->ValueToString());
				}
				else
				{
					Printf("Error: Cannot set {} to {}", foundVariable->mName, arguments[0]);
				}
			}

			return;
		}

		Printf("Command: `{}` not found.", command);
	}

	std::vector<std::string_view> FConsoleManager::FindAutoCompleteCandidates(const std::string_view command)
	{
		std::vector<std::string_view> candidates;
		candidates.reserve(mCommands.size() + mVariables.size());

		for (const auto& [name, consoleCommand] : mCommands)
		{
			if (name.starts_with(command))
			{
				candidates.push_back(name);
			}
		}

		if (command.empty() == false)
		{
			for (const auto& [name, consoleVariable] : mVariables)
			{
				if (name.starts_with(command))
				{
					candidates.push_back(name);
				}
			}
		}

		if (candidates.size() == 1 && candidates.front() == command)
		{
			candidates.clear();
		}

		return candidates;
	}
}
