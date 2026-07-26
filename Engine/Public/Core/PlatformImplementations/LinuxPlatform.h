#pragma once

#if PLATFORM_LINUX

#include "Core/PlatformImplementations/GenericPlatform.h"

namespace Turbo
{
	struct FLinuxPlatform final : public FGenericPlatform
	{
		static bool IsDebuggerPresent();

		static void Sleep(double seconds);
		static void NanoSleep(uint64 nanoSeconds);
	};
} // namespace Turbo

#endif // PLATFORM_LINUX
