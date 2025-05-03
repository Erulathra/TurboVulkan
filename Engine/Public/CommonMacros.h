#pragma once

#include "DebugTrap.h"

// Code assumes 64bit platform
#if !defined(WITH_TURBO_64) || !WITH_TURBO_64
#error C++ compiler required.
#endif // !WITH_TURBO_64

// Version macros
#define MAKE_VERSION(Major, Minor, Patch) \
    ((((uint32_t)(Major)) << 22U) | (((uint32_t)(Minor)) << 12U) | ((uint32_t)(Patch)))

#define VERSION_MAJOR(version) ((uint32_t)(version) >> 22U)
#define VERSION_MINOR(version) (((uint32_t)(version) >> 12U) & 0x3FFU)
#define VERSION_PATCH(version) ((uint32_t)(version) & 0xFFFU)

#define TURBO_VERSION() MAKE_VERSION(TURBO_VERSION_MAJOR, TURBO_VERSION_MINOR, TURBO_VERSION_PATCH)

// Debug break
#define TURBO_DEBUG_BREAK() psnip_trap()


// Assertions
#define TURBO_CHECK(CONDITION) assert(CONDITION)

#if DEBUG
namespace Turbo
{
	inline bool Ensure_Impl(bool Condition)
	{
        if (!Condition)
        {
	        TURBO_DEBUG_BREAK();
        }

		return Condition;
	}
}

#define TURBO_ENSURE(CONDITION) Ensure_Impl(CONDITION)
#else
#define TURBO_ENSURE(CONDITION) CONDITION
#endif // DEBUG

// MATH
#define TURBO_SMALL_NUMBER 1e-5f;
#define TURBO_VERY_SMALL_NUMBER 1e-8f;
