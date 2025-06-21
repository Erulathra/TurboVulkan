#pragma once

#include "Core/RHI/RHICore.h"

namespace Turbo
{
	class VulkanUtils
	{
	public:
		static void TransitionImage(const vk::CommandBuffer& commandBuffer, const vk::Image& image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout);

		static vk::Extent2D ToExtent2D(const glm::uvec2& size) { return {size.x, size.y}; }
		static vk::Extent3D ToExtent3D(const glm::uvec3& size) { return {size.x, size.y, size.z}; }
	};
} // Turbo
