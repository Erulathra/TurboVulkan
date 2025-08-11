#pragma once

#include "Core/RHI/RHICore.h"

namespace Turbo
{
	class FVulkanDevice;

	struct FPoolSizeRatio
	{
		vk::DescriptorType type;
		float ratio;
	};

	class FDescriptorAllocatorStatic
	{
	public:
		explicit FDescriptorAllocatorStatic(FVulkanDevice* device)
			: mDevice(device)
		{
			TURBO_CHECK(device);
		}

	public:
		void SetFlags(const vk::DescriptorPoolCreateFlags& newFlags) { TURBO_CHECK(!mDescriptorPool); mFlags = newFlags; }
		void Init(uint32 sets, std::span<FPoolSizeRatio> poolSizeRatios);

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

	class FDescriptorAllocatorGrowable
	{
	public:
		explicit FDescriptorAllocatorGrowable(FVulkanDevice* device)
			: mDevice(device)
			, mSetsPerPool(0)
		{
			TURBO_CHECK(device);
		}

		void Init(uint32 numSets, std::span<FPoolSizeRatio> poolSizeRatios);

		[[nodiscard]] vk::DescriptorSet Allocate(const vk::DescriptorSetLayout& layout);

		void Reset();
		void Destroy();

	private:
		[[nodiscard]] vk::DescriptorPool GetPool();
		[[nodiscard]] vk::DescriptorPool CreatePool(uint32 numSets, std::span<FPoolSizeRatio> poolSizeRatios) const;

	private:
		constexpr static uint32 kMaxSetsPerPool = 4096;

		FVulkanDevice* mDevice;

		std::vector<FPoolSizeRatio> mRatios{};
		std::vector<vk::DescriptorPool> mFullPools{};
		std::vector<vk::DescriptorPool> mReadyPools{};

		mutable uint32 mSetsPerPool;
	};
} // Turbo
