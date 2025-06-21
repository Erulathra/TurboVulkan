#include "Core/RHI/Image.h"

#include "Core/RHI/RHICore.h"
#include "Core/Engine.h"
#include "Core/RHI/RHIDestoryQueue.h"
#include "Core/RHI/VulkanDevice.h"
#include "Core/RHI/VulkanRHI.h"
#include "Core/RHI/Utils/VulkanUtils.h"

namespace Turbo
{
	class FImageDestroyer : IDestroyer
	{
	public:
		FImageDestroyer() = delete;
		explicit FImageDestroyer(const FImage& image)
			: mImage(image.GetImage())
			, mImageView(image.GetImageView())
			, mAllocation(image.GetAllocation())
		{}

		virtual ~FImageDestroyer() override = default;

	public:
		virtual void Destroy(const FVulkanDevice* device) override
		{
			if (mImageView)
			{
				device->Get().destroyImageView(mImageView);
			}

			if (mImage && mAllocation)
			{
				vmaDestroyImage(device->GetAllocator(), mImage, mAllocation);
			}
		}

	private:
		vk::Image mImage;
		vk::ImageView mImageView;
		VmaAllocation mAllocation;
	};

	FImage::FImage(FVulkanDevice& device)
		: mDevice(&device)
		, mAllocation(nullptr)
		, mSize()
		, mFormat()
	{
	}

	FImage::~FImage()
	{
		if (!bManualDestroy)
		{
			gEngine->GetRHI()->GetCurrentFrame().GetDeletionQueue().RequestDestroy(FImageDestroyer(*this));
		}
	}

	void FImage::InitResource()
	{
		TURBO_CHECK(!bResourceInitialized)

		vk::ImageCreateInfo imageCreateInfo = VulkanInitializers::Image2DCreateInfo(mFormat, mUsageFlags, VulkanUtils::ToExtent2D(mSize));
		VmaAllocationCreateInfo imageAllocationInfo {};
		imageAllocationInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
		imageAllocationInfo.requiredFlags = static_cast<VkMemoryPropertyFlags>(vk::MemoryPropertyFlagBits::eDeviceLocal);

		VkImage tempImage{};
		CHECK_VULKAN(vmaCreateImage(mDevice->GetAllocator(), imageCreateInfo, &imageAllocationInfo, &tempImage, &mAllocation, nullptr));
		mImage = tempImage;

		const vk::ImageViewCreateInfo imageViewCreateInfo = VulkanInitializers::ImageView2DCreateInfo(mFormat, mImage, vk::ImageAspectFlagBits::eColor);

		vk::Result result;
		std::tie(result, mImageView) = mDevice->Get().createImageView(imageViewCreateInfo, nullptr);
		CHECK_VULKAN_HPP(result);
	}

	void FImage::RequestDestroy(FRHIDestroyQueue& deletionQueue)
	{
		bManualDestroy = true;
		deletionQueue.RequestDestroy(FImageDestroyer(*this));

		mImage = nullptr;
		mImageView = nullptr;
		mAllocation = nullptr;
		bResourceInitialized = false;
	}

	void FImage::SetSize(const glm::ivec2& size)
	{
		TURBO_CHECK(!bResourceInitialized);
		mSize = size;
	}

	void FImage::SetFormat(const vk::Format format)
	{
		TURBO_CHECK(!bResourceInitialized);
		mFormat = format;
	}

	void FImage::SetUsage(vk::ImageUsageFlags usage)
	{
		TURBO_CHECK(!bResourceInitialized);
		mUsageFlags = usage;
	}
} // Turbo
