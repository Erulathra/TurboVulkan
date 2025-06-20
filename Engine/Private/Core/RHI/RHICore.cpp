#include "Core/RHI/RHICore.h"

#define VMA_DYNAMIC_VULKAN_FUNCTIONS 0
#define VMA_STATIC_VULKAN_FUNCTIONS 0

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wnullability-completeness"
#endif

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

#ifdef __clang__
#pragma clang diagnostic pop
#endif


fmt::format_context::iterator fmt::formatter<vk::Result>::format(vk::Result result, format_context& ctx) const
{
	return formatter<int32>::format(static_cast<int32>(result), ctx);
}

fmt::format_context::iterator fmt::formatter<VkResult>::format(VkResult result, format_context& ctx) const
{
	return formatter<int32>::format(result, ctx);
}

