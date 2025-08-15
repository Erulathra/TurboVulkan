#include "Core/RHI/Pipelines/DescriptorWriter.h"

#include "Core/RHI/Image.h"
#include "Core/RHI/VulkanDevice.h"

namespace Turbo {
	FDescriptorWriter& FDescriptorWriter::WriteSampledImage(uint32 binding, const std::shared_ptr<FImage>& image)
	{
		return WriteImage(binding, image, vk::DescriptorType::eSampledImage);
	}

	FDescriptorWriter& FDescriptorWriter::WriteStorageImage(uint32 binding, const std::shared_ptr<FImage>& image)
	{
		return WriteImage(binding, image, vk::DescriptorType::eStorageImage);
	}

	FDescriptorWriter& FDescriptorWriter::WriteSampler(uint32 binding, vk::Sampler sampler)
	{
		mSamplerBindings.emplace_back(binding, sampler);
		return *this;
	}

	FDescriptorWriter& FDescriptorWriter::WriteBuffer(uint32 binding, vk::Buffer buffer, size_t size, size_t offset, vk::DescriptorType type)
	{
		mBufferBindings.emplace_back(binding, buffer, size, offset, type);
		return *this;
	}

	FDescriptorWriter& FDescriptorWriter::WriteImage(uint32 binding, const std::shared_ptr<FImage>& image, vk::DescriptorType type)
	{
		mImageBindings.emplace_back(binding, image->GetImageView(), image->GetLayout(), type);
		return *this;
	}

	FDescriptorWriter& FDescriptorWriter::UpdateSet(FVulkanDevice* device, vk::DescriptorSet set)
	{
		std::vector<vk::WriteDescriptorSet> writes;

		std::vector<vk::DescriptorImageInfo> imageInfos;
		imageInfos.resize(mImageBindings.size());
		for (const FImageBindingData& binding : mImageBindings)
		{
			vk::DescriptorImageInfo& newImageInfo = imageInfos.emplace_back(nullptr, binding.image, binding.layout);
			vk::WriteDescriptorSet& newWrite = writes.emplace_back();
			newWrite.setDstBinding(binding.index);
			newWrite.setDstSet(set);
			newWrite.setDescriptorType(binding.type);
			newWrite.setImageInfo(newImageInfo);
		}

		std::vector<vk::DescriptorImageInfo> samplerInfos;
		samplerInfos.resize(mSamplerBindings.size());
		for (const FSamplerBindingData& binding : mSamplerBindings)
		{
			vk::DescriptorImageInfo& newImageInfo = imageInfos.emplace_back(binding.sampler, nullptr, vk::ImageLayout::eUndefined);
			vk::WriteDescriptorSet& newWrite = writes.emplace_back();
			newWrite.setDstBinding(binding.index);
			newWrite.setDstSet(set);
			newWrite.setDescriptorType(vk::DescriptorType::eSampler);
			newWrite.setImageInfo(newImageInfo);
		}

		std::vector<vk::DescriptorBufferInfo> bufferInfos;
		bufferInfos.resize(mBufferBindings.size());
		for (const FBufferBindingData& binding : mBufferBindings)
		{
			vk::DescriptorBufferInfo& newBufferInfo = bufferInfos.emplace_back(binding.buffer, binding.offset, binding.size);
			vk::WriteDescriptorSet& newWrite = writes.emplace_back();
			newWrite.setDstBinding(binding.index);
			newWrite.setDstSet(set);
			newWrite.setDescriptorType(binding.type);
			newWrite.setBufferInfo(newBufferInfo);
		}

		device->Get().updateDescriptorSets(writes, 0);

		return *this;
	}

	FDescriptorWriter& FDescriptorWriter::Clear()
	{
		mImageBindings.clear();
		mSamplerBindings.clear();
		mBufferBindings.clear();

		return *this;
	}
} // Turbo