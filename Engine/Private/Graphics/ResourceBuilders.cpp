#include "Graphics/ResourceBuilders.h"

constexpr std::array kSupportedDescriptors = {
	vk::DescriptorType::eSampler,
	vk::DescriptorType::eSampledImage,
	vk::DescriptorType::eStorageImage,
	vk::DescriptorType::eUniformBuffer,
	vk::DescriptorType::eStorageBuffer,
};

Turbo::FBufferBuilder Turbo::FBufferBuilder::CreateStagingBuffer(const void* data, uint32 size)
{
	static const FName kStagingBufferName("Staging");

	FBufferBuilder result = {};
	result
		.Init(vk::BufferUsageFlagBits::eTransferSrc, EBufferFlags::CreateMapped, size)
		.SetData(data)
		.SetName(kStagingBufferName);

	return result;
}

Turbo::FBufferBuilder Turbo::FBufferBuilder::CreateStagingBuffer(uint32 size)
{
	return CreateStagingBuffer(nullptr, size);
}

Turbo::FBufferBuilder Turbo::FBufferBuilder::CreateStagingBuffer(std::span<byte> data)
{
	return CreateStagingBuffer(data.data(), data.size());
}

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
