#pragma once

#include <vulkan/vulkan.hpp>

#include "Core/RHI/Utils/VulkanInitializers.h"

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

#define CHECK_VULKAN_HPP(EXPRESSION)																					\
{																														\
	vk::Result _result = (EXPRESSION);																					\
	TURBO_CHECK_MSG(_result == vk::Result::eSuccess, "Vulkan error: {}", (_result));									\
}

#define CHECK_VULKAN_HPP_MSG(EXPRESSION, MESSAGE, ...)																						\
{																																			\
	vk::Result _result = (EXPRESSION);																										\
	TURBO_CHECK_MSG(_result == vk::Result::eSuccess, "[Vulkan error: {}] " MESSAGE, _result __VA_OPT__(,) __VA_ARGS__)						\
}

constexpr inline uint32 kDefaultVulkanTimeout = 1000000000; // 1 second
