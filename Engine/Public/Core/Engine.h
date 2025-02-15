#pragma once
#include "Window.h"

namespace Turbo
{
	enum class EWindowEvent : uint32_t;

	enum class EExitCode : int32_t
	{
		Success = 0,
	};

	class Engine
	{
	private:
		explicit Engine();

	public:
		~Engine();

	public:
		static void Init();
		static Engine* Get();

		int32_t Start(int argc, char* argv[]);
		void End();

		void RequestExit(EExitCode InExitCode = EExitCode::Success);

	private:
		void GameThreadLoop();
		void GameThreadTick();

		void HandleMainWindowEvents(EWindowEvent Event);

	private:
		static std::unique_ptr<Engine> Instance;

	private:
		WindowEventDelegate::Handle HandleMainWindowEventsHandle;

		bool bExitRequested = false;
		EExitCode ExitCode = EExitCode::Success;
	};
} // Turbo
