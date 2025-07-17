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
				device->GetAllocator().destroyImage(mImage, mAllocation);
			}
		}

	private:
		vk::Image mImage;
		vk::ImageView mImageView;
		vma::Allocation mAllocation;
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
		vma::AllocationCreateInfo imageAllocationInfo{};
		imageAllocationInfo.usage = vma::MemoryUsage::eAutoPreferDevice;
		imageAllocationInfo.requiredFlags = vk::MemoryPropertyFlagBits::eDeviceLocal;

		std::pair<vk::Image, vma::Allocation> allocationResult;
		CHECK_VULKAN_RESULT(allocationResult, mDevice->GetAllocator().createImage(imageCreateInfo, imageAllocationInfo))
		std::tie(mImage, mAllocation) = allocationResult;

		const vk::ImageViewCreateInfo imageViewCreateInfo = VulkanInitializers::ImageView2DCreateInfo(mFormat, mImage, vk::ImageAspectFlagBits::eColor);

		vk::Result result;
		std::tie(result, mImageView) = mDevice->Get().createImageView(imageViewCreateInfo, nullptr);
		CHECK_VULKAN_HPP(result);
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
		FRHIDestroyQueue& deletionQueue = gEngine->GetRHI()->GetCurrentFrame().GetDeletionQueue();
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
