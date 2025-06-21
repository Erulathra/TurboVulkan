#pragma once

#include "Core/RHI/RHICore.h"

namespace Turbo
{
	class VulkanUtils
	{
	public:
		static void TransitionImage(const vk::CommandBuffer& commandBuffer, const vk::Image& image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout);
	};
} // Turbo
