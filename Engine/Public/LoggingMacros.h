#pragma once

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG
#include "spdlog/spdlog.h"

// Log verbosity

#if DEBUG
#define LOG_VERBOSITY 0
#else
#define LOG_VERBOSITY 3
#endif

namespace Turbo
{
	enum LogVerbosity
	{
		Display = 0,
		Info,
		Warn,
		Error,
		Critical
	};
}

// Log Categories, you should specify verbosity as value
#define LOG_ENGINE		Info
#define LOG_WINDOW		Display
#define LOG_RHI			Info
#define LOG_GPU_DEVICE	Display
#define LOG_TEMP		Display
#define LOG_STREAMING	Display
#define LOG_INPUT		Display
#define LOG_SLANG		Display

#define TURBO_MIN(a,b) (((a)<(b))?(a):(b))
#define TURBO_MAX(a,b) (((a)>(b))?(a):(b))

#define TURBO_LOG(CATEGORY, VERBOSITY, ...)                                                     \
{                                                                                               \
	using namespace Turbo;																		\
	if constexpr (VERBOSITY >= LOG_VERBOSITY && VERBOSITY >= CATEGORY)							\
	{																							\
		if constexpr (VERBOSITY == Display) SPDLOG_DEBUG(__VA_ARGS__);							\
		else if constexpr (VERBOSITY == Info) SPDLOG_INFO(__VA_ARGS__);   			    		\
		else if constexpr (VERBOSITY == Warn) SPDLOG_WARN(__VA_ARGS__);							\
		else if constexpr (VERBOSITY == Error) SPDLOG_ERROR(__VA_ARGS__);						\
		else if constexpr (VERBOSITY == Critical) SPDLOG_CRITICAL(__VA_ARGS__);					\
	}																							\
}

#if DEBUG
#define BUILD_TYPE_STR "Debug"
#else
#define BUILD_TYPE_STR "Release"
#endif
