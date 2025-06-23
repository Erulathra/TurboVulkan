#pragma once

#include "Core/RHI/RHICore.h"

namespace Turbo
{
	class FVulkanDevice;

	class FDescriptorAllocator
	{
	public:
		struct FPoolSizeRatio
		{
			vk::DescriptorType type;
			float ratio;
		};

	public:
		explicit FDescriptorAllocator(FVulkanDevice* device)
			: mDevice(device)
		{
			TURBO_CHECK(device);
		}

	public:
		void Init(uint32 maxSets, std::span<FPoolSizeRatio> poolSizeRatios);

		vk::DescriptorSet Allocate(const vk::DescriptorSetLayout& layout);

		void Reset();
		void Destroy();

	private:
		FVulkanDevice* mDevice;

		vk::DescriptorPool mDescriptorPool;
	};
} // Turbo
