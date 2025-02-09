#pragma once

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
#define LOG_TEMP 1

#define TV_MIN(a,b) (((a)<(b))?(a):(b))
#define TV_MAX(a,b) (((a)>(b))?(a):(b))

#define TV_LOG(CATEGORY, VERBOSITY, ...)                                                        \
{                                                                                               \
        constexpr unsigned int FinalVerbosity = TV_MAX(CATEGORY, VERBOSITY);                    \
        if constexpr (FinalVerbosity >= LOG_VERBOSITY)                                          \
        {                                                                                       \
            if constexpr (FinalVerbosity == LOG_DISPLAY) SPDLOG_DEBUG(__VA_ARGS__);             \
            else if constexpr (FinalVerbosity == LOG_INFO) SPDLOG_INFO(__VA_ARGS__);            \
            else if constexpr (FinalVerbosity == LOG_WARN) SPDLOG_WARN(__VA_ARGS__);            \
            else if constexpr (FinalVerbosity == LOG_ERROR) SPDLOG_ERROR(__VA_ARGS__);          \
            else if constexpr (FinalVerbosity == LOG_CRITICAL) SPDLOG_CRITICAL(__VA_ARGS__);    \
        }                                                                                       \
}

#if DEBUG
#define BUILD_TYPE_STR "Debug"
#else
#define BUILD_TYPE_STR "Release"
#endif
