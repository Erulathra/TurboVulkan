#pragma once

#if PLATFORM_WINDOWS

#include "Core/PlatformImplementations/GenericPlatform.h"

namespace Turbo
{
	struct FWindowsPlatform final : public FGenericPlatform
	{
		static bool IsDebuggerPresent();

		static void Sleep(double seconds);
	};
} // namespace Turbo

#endif // PLATFORM_WINDOWS
