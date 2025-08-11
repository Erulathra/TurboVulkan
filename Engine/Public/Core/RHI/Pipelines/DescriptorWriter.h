#pragma once

#include "Core/RHI/RHICore.h"

namespace Turbo
{
	class FVulkanDevice;
	class FImage;

	class FDescriptorWriter
	{
	public:
		FDescriptorWriter& WriteSampledImage(uint32 binding, const std::shared_ptr<FImage>& image);
		FDescriptorWriter& WriteStorageImage(uint32 binding, const std::shared_ptr<FImage>& image);

		FDescriptorWriter& WriteSampler(uint32 binding, vk::Sampler sampler);

		FDescriptorWriter& WriteBuffer(uint32 binding, vk::Buffer buffer, size_t size, size_t offset, vk::DescriptorType type);

		FDescriptorWriter& UpdateSet(FVulkanDevice* device, vk::DescriptorSet set);
		FDescriptorWriter& Clear();

	private:
		struct FImageBindingData
		{
			uint32 index;
			vk::ImageView image;
			vk::ImageLayout layout;
			vk::DescriptorType type;
		};

		struct FSamplerBindingData
		{
			uint32 index;
			vk::Sampler sampler;
		};

		struct FBufferBindingData
		{
			uint32 index;
			vk::Buffer buffer;
			size_t size;
			size_t offset;
			vk::DescriptorType type;;
		};

	private:
		FDescriptorWriter& WriteImage(uint32 binding, const std::shared_ptr<FImage>& image, vk::DescriptorType type);

	private:
		std::vector<FImageBindingData> mImageBindings{};
		std::vector<FSamplerBindingData> mSamplerBindings{};
		std::vector<FBufferBindingData> mBufferBindings{};
	};
} // Turbo
