#pragma once

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG
#include "spdlog/spdlog.h"

// Log verbosity

#if DEBUG
#define LOG_VERBOSITY 0
#else
#define LOG_VERBOSITY 3
#endif

#define LOG_DISPLAY 0
#define LOG_INFO 1
#define LOG_WARN 2
#define LOG_ERROR 3
#define LOG_CRITICAL 4

// Log Categories, you should specify verbosity as value
#define LOG_ENGINE		LOG_INFO
#define LOG_WINDOW		LOG_DISPLAY
#define LOG_RHI			LOG_INFO
#define LOG_TEMP		LOG_DISPLAY
#define LOG_STREAMING	LOG_DISPLAY

#define TURBO_MIN(a,b) (((a)<(b))?(a):(b))
#define TURBO_MAX(a,b) (((a)>(b))?(a):(b))

#define TURBO_LOG(CATEGORY, VERBOSITY, ...)                                                     \
{                                                                                               \
        if constexpr (VERBOSITY >= LOG_VERBOSITY && VERBOSITY >= CATEGORY)                      \
        {                                                                                       \
            if constexpr (VERBOSITY == LOG_DISPLAY) SPDLOG_DEBUG(__VA_ARGS__);             \
            else if constexpr (VERBOSITY == LOG_INFO) SPDLOG_INFO(__VA_ARGS__);            \
            else if constexpr (VERBOSITY == LOG_WARN) SPDLOG_WARN(__VA_ARGS__);            \
            else if constexpr (VERBOSITY == LOG_ERROR) SPDLOG_ERROR(__VA_ARGS__);          \
            else if constexpr (VERBOSITY == LOG_CRITICAL) SPDLOG_CRITICAL(__VA_ARGS__);    \
        }                                                                                       \
}

#if DEBUG
#define BUILD_TYPE_STR "Debug"
#else
#define BUILD_TYPE_STR "Release"
#endif
