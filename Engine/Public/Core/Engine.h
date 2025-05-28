#pragma once
#include "Window.h"

namespace Turbo
{
	class FVulkanRHI;
	class CommandLineArgsParser;
}

namespace Turbo
{
	enum class EWindowEvent : uint32_t;

	enum class EExitCode : int32_t
	{
		Success = 0,
		WindowCreationError,
		RHICriticalError,
		DeviceNotSupported
	};

	class FEngine
	{
	private:
		explicit FEngine();

		/** Services */
	public:
		[[nodiscard]] FVulkanRHI* GetRHI() const { return RHIInstance.get(); };
		[[nodiscard]] FSDLWindow* GetWindow() const { return MainWindowInstance.get(); };

	private:
		// Replace me with GenericRHI
		std::unique_ptr<FVulkanRHI> RHIInstance;
		std::unique_ptr<FSDLWindow> MainWindowInstance;

		/** Services end */

	public:
		~FEngine();

	public:
		static void Init();

		int32_t Start(int32 argc, char* argv[]);
		void End();

		void RequestExit(EExitCode InExitCode = EExitCode::Success);

	private:
		void GameThreadLoop();
		void GameThreadTick();

		void HandleMainWindowEvents(EWindowEvent Event);

	private:
		WindowEventDelegate::Handle HandleMainWindowEventsHandle;

		bool bExitRequested = false;
		EExitCode ExitCode = EExitCode::Success;
	};

	inline std::unique_ptr<FEngine> gEngine(nullptr);
} // Turbo
