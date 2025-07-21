#pragma once

#include "RHICore.h"

namespace Turbo
{
	class FVulkanDevice;
	class FRHIDestroyQueue;

	class FImage
	{
		GENERATED_BODY(FImage)
	private:
		explicit FImage(FVulkanDevice* device);

	public:
		FImage() = delete;
		FImage(FImage& other) = delete;
		FImage& operator=(const FImage& other) = delete;
		FImage(const FImage& other) = delete;

	public:
		static std::unique_ptr<FImage> CreateUnique(FVulkanDevice* device, glm::ivec2 size, vk::Format format, vk::ImageUsageFlags flags);
		static std::shared_ptr<FImage> CreateShared(FVulkanDevice* device, glm::ivec2 size, vk::Format format, vk::ImageUsageFlags flags);

		~FImage();

		void RequestDestroy(FRHIDestroyQueue& deletionQueue);
		void Destroy();

	public:
		[[nodiscard]] const vk::Image& GetImage() const { return mImage; }
		[[nodiscard]] const vk::ImageView& GetImageView() const { return mImageView; }
		[[nodiscard]] vma::Allocation GetAllocation() const { return mAllocation; }

		[[nodiscard]] const glm::ivec2& GetSize() const { return mSize; }
		[[nodiscard]] vk::Format GetFormat() const { return mFormat; }

	private:
		void InitResource(glm::ivec2 size, vk::Format format, vk::ImageUsageFlags flags);

	private:
		FVulkanDevice* mDevice;

		vk::Image mImage;
		vk::ImageView mImageView;
		vma::Allocation mAllocation{};

		// bool bResourceInitialized = false;
		bool bManualDestroy = false;

		glm::ivec2 mSize{};
		vk::Format mFormat;
		vk::ImageUsageFlags mUsageFlags;
	};

} // Turbo
