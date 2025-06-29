#include "Core/RHI/Pipelines/DescriptorAllocator.h"

#include "Core/RHI/VulkanDevice.h"

namespace Turbo {
	void FDescriptorAllocator::Init(uint32 maxSets, std::span<FPoolSizeRatio> poolSizeRatios)
	{
		std::vector<vk::DescriptorPoolSize> poolSizes;
		for (const FPoolSizeRatio& ratio : poolSizeRatios)
		{
			poolSizes.emplace_back(ratio.type, ratio.ratio * maxSets);
		}

		vk::DescriptorPoolCreateInfo createInfo {};
		createInfo.setPNext(nullptr);

		createInfo.flags = mFlags;
		createInfo.setMaxSets(maxSets);
		createInfo.setPoolSizes(poolSizes);

		TURBO_CHECK(mDevice)

		vk::Result result;
		std::tie(result, mDescriptorPool) = mDevice->Get().createDescriptorPool(createInfo);
		CHECK_VULKAN_HPP(result);
	}

	vk::DescriptorSet FDescriptorAllocator::Allocate(const vk::DescriptorSetLayout& layout)
	{
		TURBO_CHECK(mDevice && mDescriptorPool);

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

	void FDescriptorAllocator::Reset()
	{
		TURBO_CHECK(mDevice)
		mDevice->Get().resetDescriptorPool(mDescriptorPool);
	}

	void FDescriptorAllocator::Destroy()
	{
		TURBO_CHECK(mDevice)
		mDevice->Get().destroyDescriptorPool(mDescriptorPool);
	}
} // Turbo