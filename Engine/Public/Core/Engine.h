#pragma once
#include "Window.h"
#include "Input/Input.h"

namespace Turbo
{
	class FWorld;
	class FAssetManager;
	class FGPUDevice;
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
		[[nodiscard]] FWorld* GetWorld() const { return mWorld.get(); }

	private:
		TSharedPtr<FWorld> mWorld;

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

		void RegisterEngineLayers();

		// TODO: Move me somewhere else
		void SetupBasicInputBindings();

		void HandleMainWindowEvents(EWindowEvent event);

	private:
		bool mbExitRequested = false;
		EExitCode mExitCode = EExitCode::Success;

		EEngineState mEngineState = EEngineState::Undefined;
	};

	inline TUniquePtr<FEngine> gEngine;
} // Turbo
