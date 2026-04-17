#include "Layers/ConsoleFrontendLayer.h"

#include "imgui.h"
#include "Core/Input/Input.h"
#include "Core/Input/Keys.h"
#include "Debug/IConsoleManager.h"
#include "misc/cpp/imgui_stdlib.h"

namespace Turbo
{
	const FName kToggleConsoleName = FName("ToggleConsole");

	FName FConsoleFrontendLayer::GetName()
	{
		static FName name("ConsoleFrontend");
		return name;
	}

	void FConsoleFrontendLayer::Start()
	{
		IInputSystem& inputSystem = entt::locator<IInputSystem>::value();
		inputSystem.RegisterBinding({kToggleConsoleName, EKeys::Grave});

		IConsoleManager& consoleManager = entt::locator<IConsoleManager>::value();
		consoleManager.RegisterCommand(FConsoleCommand(
			"history",
			"Shows command history",
			FConsoleCommandDelegate::CreateLambda([this](IConsoleManager& consoleManager, const FArgsVector args)
			{
				std::string message;
				for (int32 commandId = 0; commandId < mConsoleHistory.size(); ++commandId)
				{
					message += fmt::format("\t{}\t{}\n", commandId, mConsoleHistory[commandId]);
				}

				consoleManager.Print(message);
			})
		));
	}

	void FConsoleFrontendLayer::Shutdown()
	{
		IConsoleManager& consoleManager = entt::locator<IConsoleManager>::value();
		consoleManager.UnregisterCommand("history");
	}


	int32 OnConsoleInputCallback(ImGuiInputTextCallbackData* data)
	{
		FConsoleFrontendLayer* frontend = static_cast<FConsoleFrontendLayer*>(data->UserData);
		IConsoleManager& consoleManager = entt::locator<IConsoleManager>::value();

		switch (data->EventFlag)
		{
		case ImGuiInputTextFlags_CallbackCompletion:
			{
				const std::vector<std::string_view> candidates =
					consoleManager.FindAutoCompleteCandidates(std::string_view(data->Buf, data->BufTextLen));
				if (candidates.size() == 1)
				{
					const char* begin = &candidates[0].front() + data->BufTextLen;
					const char* end = &candidates[0].back();
					data->InsertChars(data->BufTextLen, begin, end + 1);
				}
				else if (candidates.size() > 1)
				{
					std::string message;
					for (const std::string_view& candidate : candidates)
					{
						message += candidate;
						message += " ";
					}

					consoleManager.Print(message);
				}
				break;
			}
		case ImGuiInputTextFlags_CallbackHistory:
			{
				const std::vector<std::string>& history = frontend->mConsoleHistory;
				if (frontend->mHistoryIndex == history.size())
				{
					frontend->mTempBuffer = std::string_view(data->Buf, data->BufTextLen);
				}

				int32 newHistoryIndex = frontend->mHistoryIndex;
				if (data->EventKey == ImGuiKey_UpArrow)
				{
					newHistoryIndex--;
				}
				else if (data->EventKey == ImGuiKey_DownArrow)
				{
					newHistoryIndex++;
				}

				frontend->mHistoryIndex = FMath::Modulo<int32>(newHistoryIndex, history.size() + 1);

				std::string_view historyEntry;
				if (frontend->mHistoryIndex == history.size())
				{
					historyEntry = frontend->mTempBuffer;
				}
				else
				{
					historyEntry = history[frontend->mHistoryIndex];
				}

				if (historyEntry.size() > 0)
				{
					const char* begin = &historyEntry.front();
					const char* end = &historyEntry.back();

					data->DeleteChars(0, data->BufTextLen);
					data->InsertChars(0, begin, end + 1);
				}

				break;
			}
		default: ;
		}

		return 0;
	}

	void FConsoleFrontendLayer::BeginTick(double deltaTime)
	{
		ILayer::BeginTick(deltaTime);

		std::string inputBuffer;

		ImGui::Begin("Console");
		ImGui::PushItemWidth(-1);

		ImVec2 regionSize = ImGui::GetContentRegionAvail();
		regionSize.y -= ImGui::GetTextLineHeightWithSpacing() + (2.f * ImGui::GetStyle().FramePadding.y);
		ImGui::InputTextMultiline("##ConsoleOutput", &mConsoleBuffer, regionSize, ImGuiInputTextFlags_ReadOnly);

		constexpr ImGuiInputTextFlags inputTextFlags =
			ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_CallbackHistory | ImGuiInputTextFlags_EnterReturnsTrue;

		if (ImGui::InputText("##ConsoleInput", &inputBuffer, inputTextFlags, OnConsoleInputCallback, this))
		{
			IConsoleManager& consoleManager = entt::locator<IConsoleManager>::value();
			consoleManager.Printf("$ {}", inputBuffer);
			mConsoleHistory.push_back(inputBuffer);
			consoleManager.Parse(inputBuffer);

			mHistoryIndex = mConsoleHistory.size();
			mbFocusConsoleInput = true;
		}
		ImGui::PopItemWidth();

		if (mbFocusConsoleInput)
		{
			ImGui::SetKeyboardFocusHere(-1);
			mbFocusConsoleInput = false;
		}

		ImGui::End();
	}

	bool FConsoleFrontendLayer::ShouldTick()
	{
		return mbConsoleVisible;
	}

	void FConsoleFrontendLayer::OnEvent(FEventBase& event)
	{
		FEventDispatcher::Dispatch<FActionEvent>(event, this, &FConsoleFrontendLayer::HandleInputActionEvent);
		FEventDispatcher::Dispatch<FConsoleBufferChangedEvent>(event, this, &FConsoleFrontendLayer::HandleConsoleBufferChangedEvent);
	}

	void FConsoleFrontendLayer::HandleInputActionEvent(FActionEvent& event)
	{
		if (event.mActionName == kToggleConsoleName && event.mbDown)
		{
			mbConsoleVisible = !mbConsoleVisible;
			if (mbConsoleVisible)
			{
				mbFocusConsoleInput = true;
			}
		}
	}

	void FConsoleFrontendLayer::HandleConsoleBufferChangedEvent(FConsoleBufferChangedEvent& event)
	{
		mConsoleBuffer += event.mMessage;
		mConsoleBuffer += "\n";
	}
}
