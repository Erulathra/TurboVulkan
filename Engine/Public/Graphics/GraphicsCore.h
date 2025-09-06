#pragma once

#include "CommonTypeDefs.h"
#include "Core/Math/MathCommon.h"

/** Constants */

inline constexpr uint16 kInvalidSetIndex = std::numeric_limits<uint16>::max();

inline constexpr uint8 kMaxImageOutputs = 8;
inline constexpr uint8 kMaxShaderStages = 5;
inline constexpr uint8 kMaxDescriptorsPerSet = 8;
inline constexpr uint32 kMaxSwapChainImages = 5;
inline constexpr uint32 kMaxBufferedFrames = 2;

// In theory VK supports up to 16, but I want to save some memory.
inline constexpr uint8 kMaxDescriptorSetLayouts = 4;
inline constexpr uint32 kMaxDescriptorSets = 4;

/** Constants end */

#define VULKAN_HPP_ASSERT_ON_RESULT {}
#include <vulkan/vulkan.hpp>

#include "vk_mem_alloc.hpp"

template <>
struct fmt::formatter<vk::Result> : formatter<int32>
{
	format_context::iterator format(vk::Result result, format_context& ctx) const;
};

template <>
struct fmt::formatter<VkResult> : formatter<int32>
{
	format_context::iterator format(VkResult result, format_context& ctx) const;
};

#define CHECK_VULKAN(EXPRESSION) TURBO_CHECK_MSG((EXPRESSION) == VK_SUCCESS, "Vulkan Error: {}", EXPRESSION)
#define CHECK_VULKAN_MSG(EXPRESSION, MESSAGE, ...) TURBO_CHECK_MSG((EXPRESSION) == VK_SUCCESS, "[Vulkan error: {}] " MESSAGE, EXPRESSION __VA_OPT__(,) __VA_ARGS__)

#if DEBUG
#define CHECK_VULKAN_HPP(EXPRESSION)																					\
{																														\
	vk::Result _result = (EXPRESSION);																					\
	if (_result == vk::Result::eSuboptimalKHR)																			\
	{																													\
		static bool _subOptimalShown = false;																			\
		if (!_subOptimalShown)																							\
		{																												\
			SPDLOG_WARN("Suboptimal result");																			\
			TURBO_DEBUG_BREAK();																						\
			_subOptimalShown = true;																					\
		}																												\
	}																													\
	else																												\
	{																													\
		TURBO_CHECK_MSG(_result == vk::Result::eSuccess, "Vulkan error: {}", (_result));								\
	}																													\
}
#else // DEBUG
#define CHECK_VULKAN_HPP(EXPRESSION)																					\
{																														\
	vk::Result _result = (EXPRESSION);																					\
	TURBO_CHECK_MSG(_result == vk::Result::eSuccess, "Vulkan error: {}", (_result));									\
}
#endif // else DEBUG

#define CHECK_VULKAN_RESULT(VALUE, EXPRESSION)																				\
{																														\
	vk::Result _resultNew;																								\
	std::tie(_resultNew, VALUE) = EXPRESSION;																			\
	CHECK_VULKAN_HPP(_resultNew);																						\
}

#if !TURBO_BUILD_SHIPPING
#define CHECK_VULKAN_HPP_MSG(EXPRESSION, MESSAGE, ...)																						\
{																																			\
	vk::Result _result = (EXPRESSION);																										\
	if (_result == vk::Result::eSuboptimalKHR)																								\
	{																																		\
		static bool _subOptimalShown = false;																								\
		if (!_subOptimalShown)																												\
		{																																	\
			SPDLOG_WARN("Suboptimal result");																								\
			_subOptimalShown = true;																										\
		}																																	\
	}																																		\
	else																																	\
	{																																		\
		TURBO_CHECK_MSG(_result == vk::Result::eSuccess, "[Vulkan error: {}] " MESSAGE, _result __VA_OPT__(,) __VA_ARGS__);					\
	}																																		\
}
#else
#define CHECK_VULKAN_HPP_MSG(EXPRESSION, MESSAGE, ...) { (void)(EXPRESSION); }
#endif // else TURBO_BUILD_SHIPPING

constexpr inline uint32 kDefaultTimeout = 1000000000; // 1 second
constexpr inline uint32 kMaxTimeout = std::numeric_limits<uint32>::max();
