#include "Core/PlatformImplementations/WindowsPlatform.h"

#if PLATFORM_WINDOWS

#include "windows.h"

namespace Turbo
{
	bool FWindowsPlatform::IsDebuggerPresent()
	{
	   return ::IsDebuggerPresent();
	}

	void FWindowsPlatform::Sleep(double seconds)
	{
		::Sleep(static_cast<DWORD>(seconds * 1000.f));
	}
} // namespace Turbo

#endif // PLATFORM_WINDOWS
