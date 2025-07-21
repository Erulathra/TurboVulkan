#pragma once
#include "Window.h"

namespace Turbo
{
	class FCoreTimer;
	class FVulkanRHI;
	class CommandLineArgsParser;

	enum class EWindowEvent : uint32_t;

	enum class EExitCode : int32_t
	{
		Success = 0,
		WindowCreationError,
		RHICriticalError,
		DeviceNotSupported
	};

	enum class EEngineState : int32_t
	{
		Undefined = 0,
		Initializing,
		Running,
		Finalizing
	};

	class FEngine
	{
		GENERATED_BODY(FEngine)

	private:
		explicit FEngine();

		/** Services */
	public:
		[[nodiscard]] FVulkanRHI* GetRHI() const { return mRHIInstance.get(); };
		[[nodiscard]] FSDLWindow* GetWindow() const { return mMainWindowInstance.get(); };

		[[nodiscard]] FCoreTimer* GetTimer() const { return mCoreTimer.get(); }

	private:
		// Replace us with GenericRHI
		std::unique_ptr<FVulkanRHI> mRHIInstance;
		std::unique_ptr<FSDLWindow> mMainWindowInstance;

		std::unique_ptr<FCoreTimer> mCoreTimer;

		/** Services end */

	public:
		~FEngine();

	public:
		static void Init();

		int32_t Start(int32 argc, char* argv[]);
		void End();

		void RequestExit(EExitCode InExitCode = EExitCode::Success);

	public:
		[[nodiscard]] EEngineState GetEngineState() { return mEngineState; }

	private:
		void GameThreadLoop();
		void GameThreadTick();

		void HandleMainWindowEvents(EWindowEvent event);

	private:
		bool mbExitRequested = false;
		EExitCode mExitCode = EExitCode::Success;

		EEngineState mEngineState = EEngineState::Undefined;
	};

	inline std::unique_ptr<FEngine> gEngine(nullptr);
} // Turbo
