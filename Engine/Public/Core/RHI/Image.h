#pragma once

#include "RHICore.h"

namespace Turbo
{
	class FVulkanDevice;
	class FRHIDestroyQueue;

	class FImage
	{
	public:
		explicit FImage(FVulkanDevice& device);
		~FImage();

		void InitResource();
		void RequestDestroy(FRHIDestroyQueue& deletionQueue);
		void Destroy();

	public:

		[[nodiscard]] const vk::Image& GetImage() const { return mImage; }
		[[nodiscard]] const vk::ImageView& GetImageView() const { return mImageView; }
		[[nodiscard]] VmaAllocation GetAllocation() const { return mAllocation; }

		[[nodiscard]] const glm::ivec2& GetSize() const { return mSize; }
		[[nodiscard]] vk::Format GetFormat() const { return mFormat; }

		void SetSize(const glm::ivec2& size);
		void SetFormat(vk::Format format);
		void SetUsage(vk::ImageUsageFlags usage);

	private:
		FVulkanDevice* mDevice;

		vk::Image mImage;
		vk::ImageView mImageView;
		VmaAllocation mAllocation;

		bool bResourceInitialized = false;
		bool bManualDestroy = false;

		glm::ivec2 mSize;
		vk::Format mFormat;
		vk::ImageUsageFlags mUsageFlags;
	};

} // Turbo
