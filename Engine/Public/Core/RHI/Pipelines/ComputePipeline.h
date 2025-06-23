#pragma once

#include "Core/RHI/RHICore.h"

namespace Turbo
{
	class FDescriptorAllocator;
	class FVulkanDevice;

	class FComputePipeline
	{
	public:
		explicit FComputePipeline(FVulkanDevice* device)
			: mDevice(device)
		{
		}

		void SetDescriptors(const vk::DescriptorSetLayout& layout, const vk::DescriptorSet& set);

	private:
		FVulkanDevice* mDevice;

		vk::Pipeline mPipeline = nullptr;
		vk::PipelineLayout mPipelineLayout = nullptr;

		vk::DescriptorSetLayout mDescriptorSetLayout = nullptr;
		vk::DescriptorSet mDescriptorSet = nullptr;
	};
} // Turbo
