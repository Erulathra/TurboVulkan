#include "Core/PlatformImplementations/GenericPlatform.h"

#include <emmintrin.h>

namespace Turbo
{
	void FGenericPlatform::YieldCPU()
	{
	   _mm_pause();
	}
} // namespace Turbo
