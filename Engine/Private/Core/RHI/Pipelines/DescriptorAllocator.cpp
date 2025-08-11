#include "Core/RHI/Pipelines/DescriptorAllocator.h"

#include "Core/RHI/VulkanDevice.h"

namespace Turbo {
	void FDescriptorAllocatorStatic::Init(uint32 sets, std::span<FPoolSizeRatio> poolSizeRatios)
	{
		std::vector<vk::DescriptorPoolSize> poolSizes;
		for (const FPoolSizeRatio& ratio : poolSizeRatios)
		{
			poolSizes.emplace_back(ratio.type, ratio.ratio * sets);
		}

		vk::DescriptorPoolCreateInfo createInfo {};
		createInfo.setPNext(nullptr);

		createInfo.flags = mFlags;
		createInfo.setMaxSets(sets);
		createInfo.setPoolSizes(poolSizes);

		vk::Result result;
		std::tie(result, mDescriptorPool) = mDevice->Get().createDescriptorPool(createInfo);
		CHECK_VULKAN_HPP(result);
	}

	vk::DescriptorSet FDescriptorAllocatorStatic::Allocate(const vk::DescriptorSetLayout& layout)
	{
		vk::DescriptorSetAllocateInfo allocInfo {};
		allocInfo.setPNext(nullptr);

		allocInfo.setDescriptorPool(mDescriptorPool);
		allocInfo.setSetLayouts(layout);

		vk::Result result;
		std::vector<vk::DescriptorSet> descriptorSet;
		std::tie(result, descriptorSet) = mDevice->Get().allocateDescriptorSets(allocInfo);
		CHECK_VULKAN_HPP(result);

		return descriptorSet[0];
	}

	void FDescriptorAllocatorStatic::Reset()
	{
		mDevice->Get().resetDescriptorPool(mDescriptorPool);
	}

	void FDescriptorAllocatorStatic::Destroy()
	{
		mDevice->Get().destroyDescriptorPool(mDescriptorPool);
	}

	void FDescriptorAllocatorGrowable::Init(uint32 numSets, std::span<FPoolSizeRatio> poolSizeRatios)
	{
		mRatios.clear();
		mRatios.append_range(poolSizeRatios);

		TURBO_CHECK(numSets < kMaxSetsPerPool);

		mReadyPools.push_back(CreatePool(numSets, poolSizeRatios));
	}

	vk::DescriptorSet FDescriptorAllocatorGrowable::Allocate(const vk::DescriptorSetLayout& layout)
	{
		vk::DescriptorPool targetPool = GetPool();

		vk::DescriptorSetAllocateInfo allocateInfo{};
		allocateInfo.descriptorPool = targetPool;
		allocateInfo.descriptorSetCount = 1;
		allocateInfo.pSetLayouts = &layout;

		vk::Result allocationResult;
		std::vector<vk::DescriptorSet> newSets;
		std::tie(allocationResult, newSets) = mDevice->Get().allocateDescriptorSets(allocateInfo);

		if (allocationResult == vk::Result::eErrorOutOfPoolMemory || allocationResult == vk::Result::eErrorFragmentedPool)
		{
			mFullPools.push_back(targetPool);

			// Try once more
			targetPool = GetPool();
			CHECK_VULKAN_RESULT(newSets, mDevice->Get().allocateDescriptorSets(allocateInfo));
		}

		mReadyPools.push_back(targetPool);
		return newSets.back();
	}

	void FDescriptorAllocatorGrowable::Reset()
	{
		for (const vk::DescriptorPool& pool : mReadyPools)
		{
			mDevice->Get().resetDescriptorPool(pool);
		}

		for (const vk::DescriptorPool& pool : mFullPools)
		{
			mDevice->Get().resetDescriptorPool(pool);
			mReadyPools.push_back(pool);
		}

		mFullPools.clear();
	}

	void FDescriptorAllocatorGrowable::Destroy()
	{
		for (const vk::DescriptorPool& pool : mReadyPools)
		{
			mDevice->Get().destroy(pool);
		}

		for (const vk::DescriptorPool& pool : mFullPools)
		{
			mDevice->Get().destroy(pool);
		}

		mReadyPools.clear();
		mFullPools.clear();
	}

	vk::DescriptorPool FDescriptorAllocatorGrowable::GetPool()
	{
		vk::DescriptorPool result;
		if (!mReadyPools.empty())
		{
			result = mReadyPools.back();
			mReadyPools.pop_back();
		}
		else
		{
			result = CreatePool(mSetsPerPool, mRatios);
		}

		return result;
	}

	vk::DescriptorPool FDescriptorAllocatorGrowable::CreatePool(uint32 numSets, std::span<FPoolSizeRatio> poolSizeRatios) const
	{
		std::vector<vk::DescriptorPoolSize> poolSizes;
		for (const FPoolSizeRatio& ratio : poolSizeRatios)
		{
			poolSizes.emplace_back(ratio.type, static_cast<uint32>(ratio.ratio * static_cast<float>(numSets)));
		}

		vk::DescriptorPoolCreateInfo createInfo {};
		createInfo.setPNext(nullptr);

		createInfo.flags = {};
		createInfo.setMaxSets(numSets);
		createInfo.setPoolSizes(poolSizes);

		vk::DescriptorPool newPool;
		CHECK_VULKAN_RESULT(newPool, mDevice->Get().createDescriptorPool(createInfo));

		mSetsPerPool = static_cast<uint32>(static_cast<float>(mSetsPerPool) * 1.5f);
		mSetsPerPool = std::min(kMaxSetsPerPool, mSetsPerPool);

		return newPool;
	}
} // Turbo