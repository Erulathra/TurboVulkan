#include "Core/PlatformImplementations/GenericPlatform.h"

#include <emmintrin.h>

namespace Turbo
{
	void FGenericPlatform::YieldCPU()
	{
	   _mm_pause();
	}

	std::optional<std::string> FGenericPlatform::GetEnviromentalVariable(std::string_view variableName)
	{
	   const std::string nullTerminatedVariableName{variableName};
		if (const char* variableValue = std::getenv(nullTerminatedVariableName.c_str()))
		{
         return std::string(variableValue);
		}

		return {};
	}
} // namespace Turbo
