#pragma once

#include "Core/RHI/RHICore.h"
#include "Core/RHI/VulkanDevice.h"

namespace Turbo
{
	namespace VulkanUtils
	{
		void TransitionImage(const vk::CommandBuffer& cmd, const vk::Image& image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout);
		void BlitImage(const vk::CommandBuffer& cmd, const vk::Image& src, const glm::uvec2& srcSize, const vk::Image& dst, const glm::uvec2& dstSize);

		inline vk::Extent2D ToExtent2D(const glm::uvec2& size) { return {size.x, size.y}; }
		inline vk::Extent3D ToExtent3D(const glm::uvec3& size) { return {size.x, size.y, size.z}; }

		inline vk::Offset2D ToOffset2D(const glm::ivec2& size) { return vk::Offset2D{size.x, size.y}; }
		inline vk::Offset3D ToOffset3D(const glm::ivec3& size) { return vk::Offset3D{size.x, size.y, size.z}; }

		vk::Viewport CreateViewport(const glm::vec2& start, const glm::vec2& end, float depthStart = 0.f, float depthEnd = 1.f);
		vk::Rect2D CreateRect(const glm::ivec2& start, const glm::ivec2& end);

		vk::ShaderModule CreateShaderModule(const FVulkanDevice* device, const std::span<uint32>& shaderData);
	};
} // Turbo
