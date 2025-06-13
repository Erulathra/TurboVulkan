#pragma once

#include "Core/RHI/RHICore.h"

namespace Turbo
{
	class VulkanUtils
	{
	public:
		static void TransitionImage(const vk::CommandBuffer& commandBuffer, const vk::Image& image, const vk::ImageLayout oldLayout, const vk::ImageLayout newLayout);
	};
} // Turbo
