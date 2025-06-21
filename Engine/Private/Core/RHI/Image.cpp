#include "Core/RHI/Image.h"

#include "Core/Engine.h"
#include "Core/RHI/RHIDestoryQueue.h"
#include "Core/RHI/VulkanDevice.h"
#include "Core/RHI/VulkanRHI.h"

namespace Turbo
{
	class FImageDestroyer : IDestroyer
	{
	public:
		explicit FImageDestroyer(const FImage& image)
			: mImage(image.GetImage())
			, mImageView(image.GetImageView())
		{}

		virtual ~FImageDestroyer() override = default;

	public:
		virtual void Destroy(const FVulkanDevice* device) override
		{
			if (mImageView)
			{
				device->Get().destroyImageView(mImageView);
			}

			if (mImage)
			{
				device->Get().destroyImage(mImage);
			}
		}

	private:
		vk::Image mImage;
		vk::ImageView mImageView;
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

	}

	void FImage::RequestDestroy(FRHIDestroyQueue& deletionQueue)
	{
		bManualDestroy = true;
		deletionQueue.RequestDestroy(FImageDestroyer(*this));
	}

} // Turbo
