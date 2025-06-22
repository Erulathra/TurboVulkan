#pragma once

#include "Core/RHI/RHICore.h"

namespace Turbo
{
	class VulkanUtils
	{
	public:
		static void TransitionImage(const vk::CommandBuffer& cmd, const vk::Image& image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout);
		static void BlitImage(const ::vk::CommandBuffer& cmd, const ::vk::Image& src, const glm::uvec2& srcSize, const ::vk::Image& dst, const glm::uvec2& dstSize);

		static vk::Extent2D ToExtent2D(const glm::uvec2& size) { return {size.x, size.y}; }
		static vk::Extent3D ToExtent3D(const glm::uvec3& size) { return {size.x, size.y, size.z}; }

		static vk::Offset2D ToOffset2D(const glm::ivec2& size) { return vk::Offset2D{size.x, size.y}; }
		static vk::Offset3D ToOffset3D(const glm::ivec3& size) { return vk::Offset3D{size.x, size.y, size.z}; }
	};
} // Turbo
