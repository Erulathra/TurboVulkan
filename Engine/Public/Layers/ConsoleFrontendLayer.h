#pragma once

#include "Layer.h"
#include "Debug/IConsoleManager.h"

class ImGuiInputTextCallbackData;

namespace Turbo
{
	struct FConsoleBufferChangedEvent;
	struct FActionEvent;

	class FConsoleFrontendLayer : public ILayer
	{
	public:
		virtual void Start() override;
		virtual void Shutdown() override;

		virtual void OnEvent(FEventBase& event) override;

		virtual void BeginTick(double deltaTime) override;
		virtual bool ShouldTick() override;

		virtual FName GetName() override;

	private:
		void HandleInputActionEvent(FActionEvent& event);
		void HandleConsoleBufferChangedEvent(FConsoleBufferChangedEvent& event);

	private:
		std::string mConsoleBuffer;
		std::vector<std::string> mConsoleHistory;
		// Used when accessing history, as 0th entry.
		std::string mTempBuffer;

		uint32 mHistoryIndex = 0;

		bool mbConsoleVisible : 1 = false;
		bool mbFocusConsoleInput : 1 = false;

	public:
		friend int32 OnConsoleInputCallback(ImGuiInputTextCallbackData* data);
	};
}
