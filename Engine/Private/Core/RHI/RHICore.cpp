#include "Core/RHI/RHICore.h"

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

fmt::format_context::iterator fmt::formatter<vk::Result>::format(vk::Result result, format_context& ctx) const
{
	return formatter<int32>::format(static_cast<int32>(result), ctx);
}

fmt::format_context::iterator fmt::formatter<VkResult>::format(VkResult result, format_context& ctx) const
{
	return formatter<int32>::format(result, ctx);
}

