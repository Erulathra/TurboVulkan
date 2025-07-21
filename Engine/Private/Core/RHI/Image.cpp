#include "Core/RHI/Image.h"

#include "Core/RHI/RHICore.h"
#include "Core/Engine.h"
#include "Core/RHI/RHIDestoryQueue.h"
#include "Core/RHI/VulkanDevice.h"
#include "Core/RHI/VulkanRHI.h"
#include "Core/RHI/Utils/VulkanUtils.h"

namespace Turbo
{
	class FImageDestroyer : public IDestroyer
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
				device->GetAllocator().destroyImage(mImage, mAllocation);
			}
		}

	private:
		vk::Image mImage;
		vk::ImageView mImageView;
		vma::Allocation mAllocation;
	};

	FImage::FImage(FVulkanDevice* device)
		: mDevice(device)
		, mAllocation(nullptr)
		, mSize()
		, mFormat()
	{
	}

	std::unique_ptr<FImage> FImage::CreateUnique(FVulkanDevice* device, glm::ivec2 size, vk::Format format, vk::ImageUsageFlags flags)
	{
		TURBO_CHECK(device);
		std::unique_ptr<FImage> resultImage(new FImage(device));
		resultImage->InitResource(size, format, flags);

		return std::move(resultImage);
	}

	std::shared_ptr<FImage> FImage::CreateShared(FVulkanDevice* device, glm::ivec2 size, vk::Format format, vk::ImageUsageFlags flags)
	{
		TURBO_CHECK(device);
		std::shared_ptr<FImage> resultImage(new FImage(device));
		resultImage->InitResource(size, format, flags);

		return std::move(resultImage);
	}

	FImage::~FImage()
	{
		if (!bManualDestroy)
		{
			gEngine->GetRHI()->GetDeletionQueue().RequestDestroy(FImageDestroyer(*this));
		}
	}

	void FImage::RequestDestroy(FRHIDestroyQueue& deletionQueue)
	{
		bManualDestroy = true;
		deletionQueue.RequestDestroy(FImageDestroyer(*this));
	}

	void FImage::Destroy()
	{
		bManualDestroy = true;

		// TODO: Handle when engine is tearing down.
		FRHIDestroyQueue& deletionQueue = gEngine->GetRHI()->GetDeletionQueue();
		deletionQueue.RequestDestroy(FImageDestroyer(*this));

		mImage = nullptr;
		mImageView = nullptr;
		mAllocation = nullptr;
	}

	void FImage::InitResource(glm::ivec2 size, vk::Format format, vk::ImageUsageFlags flags)
	{
		mSize = size;
		mFormat = format;
		mUsageFlags = flags;

		vk::ImageCreateInfo imageCreateInfo = VulkanInitializers::Image2DCreateInfo(format, flags, VulkanUtils::ToExtent2D(size));
		vma::AllocationCreateInfo imageAllocationInfo{};
		imageAllocationInfo.usage = vma::MemoryUsage::eAutoPreferDevice;
		imageAllocationInfo.requiredFlags = vk::MemoryPropertyFlagBits::eDeviceLocal;

		std::pair<vk::Image, vma::Allocation> allocationResult;
		CHECK_VULKAN_RESULT(allocationResult, mDevice->GetAllocator().createImage(imageCreateInfo, imageAllocationInfo))
		std::tie(mImage, mAllocation) = allocationResult;

		const vk::ImageViewCreateInfo imageViewCreateInfo = VulkanInitializers::ImageView2DCreateInfo(format, mImage, vk::ImageAspectFlagBits::eColor);

		CHECK_VULKAN_RESULT(mImageView, mDevice->Get().createImageView(imageViewCreateInfo, nullptr));
	}
} // Turbo
