#pragma once

#include "RHICore.h"

namespace Turbo
{
	class FVulkanDevice;
	class FDestroyQueue;

	struct FImageCreateInfo
	{
		glm::ivec2 Size{};
		vk::Format Format{};
		vk::ImageUsageFlags UsageFlags{};
		vk::ImageAspectFlags AspectFlags = vk::ImageAspectFlagBits::eColor;
	};

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
		static std::unique_ptr<FImage> CreateUnique(FVulkanDevice* device, const FImageCreateInfo& imageCreateInfo);
		static std::shared_ptr<FImage> CreateShared(FVulkanDevice* device, const FImageCreateInfo& imageCreateInfo);

		~FImage();

		void RequestDestroy(FDestroyQueue& deletionQueue);
		void Destroy();

	public:
		[[nodiscard]] const vk::Image& GetImage() const { return mImage; }
		[[nodiscard]] const vk::ImageView& GetImageView() const { return mImageView; }
		[[nodiscard]] vma::Allocation GetAllocation() const { return mAllocation; }

		[[nodiscard]] const glm::ivec2& GetSize() const { return mSize; }
		[[nodiscard]] vk::Format GetFormat() const { return mFormat; }
		[[nodiscard]] vk::ImageLayout GetLayout() const { return mCurrentLayout; }

	public:
		void Transition(vk::CommandBuffer cmd, vk::ImageLayout newLayout, vk::ImageAspectFlags aspectFlags = vk::ImageAspectFlagBits::eColor);

	private:
		void InitResource(const FImageCreateInfo& createInfo);

	private:
		FVulkanDevice* mDevice;

		vk::Image mImage;
		vk::ImageView mImageView;
		vma::Allocation mAllocation{};

		vk::ImageLayout mCurrentLayout = vk::ImageLayout::eUndefined;

		// bool bResourceInitialized = false;
		bool bManualDestroy = false;

		glm::ivec2 mSize{};
		vk::Format mFormat;
		vk::ImageUsageFlags mUsageFlags;
	};

} // Turbo
