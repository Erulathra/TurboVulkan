#pragma once

#if PLATFORM_LINUX
#include "Core/PlatformImplementations/LinuxPlatform.h"
namespace Turbo
{
	using FPlatform = FLinuxPlatform;
}
#elif PLATFORM_WINDOWS
#include "Core/PlatformImplementations/WindowsPlatform.h"
namespace Turbo
{
	using FPlatform = FWindowsPlatform;
}
#endif // else if PLATFORM_WINDOWS
