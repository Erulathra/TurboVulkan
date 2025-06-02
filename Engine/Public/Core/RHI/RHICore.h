#pragma once

#include <vulkan/vulkan.hpp>

template <>
struct fmt::formatter<vk::Result> : formatter<string_view>
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

#define CHECK_VULKAN_HPP(EXPRESSION) TURBO_CHECK_MSG((EXPRESSION) == vk::Result::eSuccess, "Vulkan error: {}", (EXPRESSION))
#define CHECK_VULKAN_HPP_MSG(EXPRESSION, MESSAGE, ...) TURBO_CHECK_MSG((EXPRESSION) == vk::Result::eSuccess, "[Vulkan error: {}] " MESSAGE, (EXPRESSION) __VA_OPT__(,) __VA_ARGS__)
