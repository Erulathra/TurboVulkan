#include "Graphics/ResourceBuilders.h"

constexpr std::array kSupportedDescriptors = {
	vk::DescriptorType::eSampler,
	vk::DescriptorType::eSampledImage,
	vk::DescriptorType::eStorageImage,
	vk::DescriptorType::eUniformBuffer,
	vk::DescriptorType::eStorageBuffer,
};

Turbo::FDescriptorPoolBuilder::FDescriptorPoolBuilder()
{
	Reset();
}

Turbo::FDescriptorPoolBuilder& Turbo::FDescriptorPoolBuilder::Reset()
{
	mMaxSets = 0;

	for (vk::DescriptorType type : kSupportedDescriptors)
	{
		SetPoolRatio(type, 1.f);
	}

	return *this;
}

Turbo::FDescriptorPoolBuilder& Turbo::FDescriptorPoolBuilder::SetPoolRatio(vk::DescriptorType type, float ratio)
{
	mPoolSizes[type] = ratio;
	return *this;
}
