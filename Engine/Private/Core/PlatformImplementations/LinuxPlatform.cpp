#include "Core/PlatformImplementations/LinuxPlatform.h"
#include "CommonMacros.h"
#include <cmath>
#include <cstdint>
#include <ctime>
#include <unistd.h>

#if PLATFORM_LINUX

#include <time.h>
#include <stdio.h>

#include <fstream>
#include <string>

namespace Turbo
{
   constexpr uint64 kSecondsToNanoSeconds = 1000000000ull;

	bool FLinuxPlatform::IsDebuggerPresent()
	{
#if TURBO_BUILD_SHIPPING
		return false;
#else // TURBO_BUILD_SHIPPING
		std::ifstream statusFile("/proc/self/status");
		if (statusFile.is_open() == false || statusFile.good() == false)
		{
			return false;
		}

		uint32 bufferSize = 256;
		std::string buffer;
		buffer.resize(bufferSize);

		statusFile.read(buffer.data(), buffer.size());
		buffer.resize(statusFile.gcount());

		const std::string TracerPidStr = "TracerPid:";
		const size_t foundIndex = buffer.find(TracerPidStr);

		if (foundIndex != std::string::npos)
		{
			for (uint32 charIndex = foundIndex + TracerPidStr.size(); charIndex <= buffer.size(); ++charIndex)
			{
				const char character = buffer[charIndex];
				if (std::isdigit(character) != 0)
				{
					return character != '0';
				}
			}
		}

		return false;
#endif // else TURBO_BUILD_SHIPPING
	}

	void FLinuxPlatform::Sleep(double seconds)
	{
   	NanoSleep(static_cast<uint64>((seconds * static_cast<double>(kSecondsToNanoSeconds))));
	}

	void FLinuxPlatform::NanoSleep(uint64 nanoSeconds)
	{
	   timespec remaining = {};
      timespec spec = {
         .tv_sec =  static_cast<int64>(nanoSeconds / kSecondsToNanoSeconds),
         .tv_nsec = static_cast<int64>(nanoSeconds % kSecondsToNanoSeconds),
      };

		nanosleep(&spec, &remaining);
	}
} // namespace Turbo


#endif // PLATFORM_LINUX
