#pragma once

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG
#include "spdlog/spdlog.h"
#include "entt/entt.hpp"

// Log verbosity

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

#if TURBO_BUILD_DEVELOPMENT
#define LOG_VERBOSITY 0
#else
#define LOG_VERBOSITY 3
#endif

namespace Turbo
{
	template<typename LogCategoryType>
	constexpr entt::hashed_string GetLogCategoryName() { return entt::hashed_string{"LogCategory"}; }

	template<typename LogCategoryType>
	constexpr LogVerbosity GetLogCategoryStaticVerbosity() { return Display; }

	template<typename LogCategoryType>
	constexpr LogVerbosity GetLogCategoryDefaultVerbosity() { return Display; }

	extern entt::dense_map<entt::hashed_string::value_type, LogVerbosity> gLogVerbosityMap;

	template<typename LogCategoryType>
	LogVerbosity GetLogCategoryDynamicVerbosity()
	{
#if TURBO_BUILD_DEVELOPMENT
		const entt::hashed_string CategoryName = GetLogCategoryName<LogCategoryType>();
		if (auto it = gLogVerbosityMap.find(CategoryName.value());
			it != gLogVerbosityMap.end())
		{
			return it->second;
		}

		const LogVerbosity DefaultVerbosity = GetLogCategoryDefaultVerbosity<LogCategoryType>();
		gLogVerbosityMap[CategoryName] = DefaultVerbosity;
		return DefaultVerbosity;
#else // TURBO_BUILD_DEVELOPMENT
		return GetLogCategoryStaticVerbosity<LogCategoryType>();
#endif // TURBO_BUILD_DEVELOPMENT
	}
}

#define DECLARE_LOG_CATEGORY(NAME, DEFAULT_VERBOSITY, STATIC_VERBOSITY)													\
	struct NAME {};																										\
	template<>																											\
	constexpr entt::hashed_string Turbo::GetLogCategoryName<NAME>() { return entt::hashed_string{#NAME}; }				\
	template<>																											\
	constexpr Turbo::LogVerbosity Turbo::GetLogCategoryStaticVerbosity<NAME>() { return Turbo::STATIC_VERBOSITY; }		\
	template<>																											\
	constexpr Turbo::LogVerbosity Turbo::GetLogCategoryDefaultVerbosity<NAME>() { return Turbo::DEFAULT_VERBOSITY; }	\


#define TURBO_LOG(CATEGORY, VERBOSITY, ...)																\
{																											\
	using namespace Turbo;																					\
	if constexpr (VERBOSITY >= LOG_VERBOSITY && VERBOSITY >= GetLogCategoryStaticVerbosity<CATEGORY>())		\
	{																										\
		if (VERBOSITY >= GetLogCategoryDynamicVerbosity<CATEGORY>())										\
		{																									\
			if constexpr (VERBOSITY == Display) SPDLOG_DEBUG(__VA_ARGS__);									\
			else if constexpr (VERBOSITY == Info) SPDLOG_INFO(__VA_ARGS__);   			    				\
			else if constexpr (VERBOSITY == Warn) SPDLOG_WARN(__VA_ARGS__);									\
			else if constexpr (VERBOSITY == Error) SPDLOG_ERROR(__VA_ARGS__);								\
			else if constexpr (VERBOSITY == Critical) SPDLOG_CRITICAL(__VA_ARGS__);							\
		}																									\
	}																										\
}

DECLARE_LOG_CATEGORY(LogTemp, Display, Display);
DECLARE_LOG_CATEGORY(LogEngine, Display, Display);