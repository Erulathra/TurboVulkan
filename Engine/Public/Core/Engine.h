#pragma once
#include "Window.h"
#include "Input/Input.h"

namespace Turbo
{
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
		[[nodiscard]] FGPUDevice* GetGpu() const { return mGpuDevice.get(); }

		[[nodiscard]] FWindow* GetWindow() const { return mWindow.get(); }
		[[nodiscard]] IInputSystem* GetInputSystem() const { return mInputSystemInstance.get(); }

		[[nodiscard]] FCoreTimer* GetTimer() const { return mCoreTimer.get(); }

	private:
		// Replace us with GenericRHI
		std::shared_ptr<FGPUDevice> mGpuDevice;
		std::shared_ptr<FWindow> mWindow;

		std::unique_ptr<IInputSystem> mInputSystemInstance;

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

		void RegisterEngineLayers();

		// TODO: Move me somewhere else
		void SetupBasicInputBindings();

		void HandleMainWindowEvents(EWindowEvent event);

	private:
		bool mbExitRequested = false;
		EExitCode mExitCode = EExitCode::Success;

		EEngineState mEngineState = EEngineState::Undefined;
	};

	inline std::unique_ptr<FEngine> gEngine(nullptr);
} // Turbo
