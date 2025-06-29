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
		void SetFlags(const vk::DescriptorPoolCreateFlags& newFlags) { TURBO_CHECK(!mDescriptorPool); mFlags = newFlags; }
		void Init(uint32 maxSets, std::span<FPoolSizeRatio> poolSizeRatios);

		vk::DescriptorSet Allocate(const vk::DescriptorSetLayout& layout);

		void Reset();
		void Destroy();

	public:
		[[nodiscard]] vk::DescriptorPool Get() const { return mDescriptorPool; }

	private:
		FVulkanDevice* mDevice;

		vk::DescriptorPool mDescriptorPool = nullptr;
		vk::DescriptorPoolCreateFlags mFlags = {};
	};
} // Turbo
