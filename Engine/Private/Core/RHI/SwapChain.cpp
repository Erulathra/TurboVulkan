#include "Core/RHI/SwapChain.h"

#include "Core/Engine.h"
#include "Core/Window.h"
#include "Core/Math/Math.h"
#include "Core/RHI/VulkanHardwareDevice.h"
#include "Core/RHI/VulkanDevice.h"

using namespace Turbo;


FSwapChain::FSwapChain(FVulkanDevice& device)
	: mDevice(&device)
{

}

void FSwapChain::Init()
{
	TURBO_CHECK(mDevice->IsValid());

	const FVulkanHardwareDevice* hardwareDevice = mDevice->GetHardwareDevice();

	TURBO_LOG(LOG_RHI, Info, "Creating Swap chain");

	const SwapChainDeviceSupportDetails supportDetails = hardwareDevice->QuerySwapChainSupport();

	vk::SwapchainCreateInfoKHR createInfo{};
	createInfo.setSurface(gEngine->GetWindow()->GetVulkanSurface());
	vk::SurfaceFormatKHR pixelFormat = SelectBestSurfacePixelFormat(supportDetails.Formats);
	createInfo.setImageFormat(pixelFormat.format);
	createInfo.setImageColorSpace(pixelFormat.colorSpace);
	createInfo.setImageArrayLayers(1);
	createInfo.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst);
	createInfo.setPresentMode(SelectBestPresentMode(supportDetails.PresentModes));

	uint32 imageCount = supportDetails.Capabilities.minImageCount;
	if (supportDetails.Capabilities.maxImageCount > 0)
	{
		imageCount = glm::min(imageCount, supportDetails.Capabilities.maxImageCount);
	}
	createInfo.minImageCount = imageCount;
	createInfo.imageExtent = CalculateSwapChainExtent(supportDetails.Capabilities);

	const FQueueFamilyIndices& queueIndices = mDevice->GetQueueIndices();
	const std::set<uint32> uniqueQueueIndices = queueIndices.GetUniqueQueueIndices();

#if 1
		createInfo.imageSharingMode = vk::SharingMode::eExclusive;
#else
	if (queueIndices.GraphicsFamily == queueIndices.PresentFamily)
	{
		createInfo.imageSharingMode = vk::SharingMode::eExclusive;
	}
	else
	{
		createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
		createInfo.queueFamilyIndexCount = queueIndices.Num();

		const std::vector queueIndicesVector(uniqueQueueIndices.begin(), uniqueQueueIndices.end());
		createInfo.setQueueFamilyIndices(queueIndicesVector);
	}
#endif

	createInfo.preTransform = supportDetails.Capabilities.currentTransform;
	createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;

	createInfo.presentMode = SelectBestPresentMode(supportDetails.PresentModes);
	createInfo.clipped = VK_TRUE;

	createInfo.oldSwapchain = nullptr;

	vk::Result vulkanResult;
	std::tie(vulkanResult, mVulkanSwapChain) = mDevice->Get().createSwapchainKHR(createInfo);
	CHECK_VULKAN_HPP_MSG(vulkanResult, "Cannot create swapchain");

	mImageFormat = createInfo.imageFormat;
	TURBO_LOG(LOG_RHI, Info, "Swapchain color format: {}", magic_enum::enum_name(mImageFormat));

	mImageSize = createInfo.imageExtent;

	std::vector<vk::Image> outImages;
	std::tie(vulkanResult, outImages) = mDevice->Get().getSwapchainImagesKHR(mVulkanSwapChain);
	CHECK_VULKAN_HPP(vulkanResult);

	for (const vk::Image& image : outImages)
	{
		mImageDatas.push_back({});
		mImageDatas.back().Image = image;
	}

	InitializeImageViews();
	InitializeSemaphores();
}


void FSwapChain::Destroy()
{
	TURBO_LOG(LOG_RHI, Info, "Destroying Swap chain");
	TURBO_CHECK(mDevice->IsValid());

	for (const FSwapchainImageData& imageData : mImageDatas)
	{
		mDevice->Get().destroyImageView(imageData.ImageView);
		mDevice->Get().destroySemaphore(imageData.QueueSubmitSemaphore);
	}
	mImageDatas.clear();

	mDevice->Get().destroySwapchainKHR((mVulkanSwapChain));
	mImageIndex = 0;
}

bool FSwapChain::IsValid() const
{
	return mVulkanSwapChain;
}

bool FSwapChain::ResizeIfRequested()
{
	if (bResizeRequested)
	{
		TURBO_LOG(LOG_RHI, Info, "Resizing Swapchain");

		CHECK_VULKAN_HPP(mDevice->Get().waitIdle());

		Destroy();
		Init();

		bResizeRequested = false;

		return true;
	}

	return false;
}

bool FSwapChain::AcquireImage(const vk::Semaphore& acquireSemaphore)
{
	vk::Result result;
	std::tie(result, mImageIndex) = mDevice->Get().acquireNextImageKHR(mVulkanSwapChain, kDefaultVulkanTimeout, acquireSemaphore, nullptr);

	if (result == vk::Result::eErrorOutOfDateKHR)
	{
		RequestResize();
		return false;
	}

	CHECK_VULKAN_HPP_MSG(result, "Cannot obtain image from a swap chain.");
	return true;
}

void FSwapChain::PresentImage()
{
	const vk::PresentInfoKHR presentInfo = VulkanInitializers::PresentInfo(mVulkanSwapChain, mImageDatas[mImageIndex].QueueSubmitSemaphore, mImageIndex);
	const vk::Result result = mDevice->GetQueues().PresentQueue.presentKHR(presentInfo);

	if (result == vk::Result::eErrorOutOfDateKHR)
	{
		RequestResize();
		return;
	}

	CHECK_VULKAN_HPP_MSG(result, "Cannot present image.");
}

vk::SurfaceFormatKHR FSwapChain::SelectBestSurfacePixelFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats) const
{
	TURBO_CHECK(!availableFormats.empty());

	for (const vk::SurfaceFormatKHR& format : availableFormats)
	{
		if (format.format == vk::Format::eB8G8R8A8Srgb
			&& format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
		{
			return format;
		}
	}

	return availableFormats.front();
}

vk::PresentModeKHR FSwapChain::SelectBestPresentMode(const std::vector<vk::PresentModeKHR>& availableModes) const
{
	TURBO_CHECK(!availableModes.empty());

	if (std::ranges::contains(availableModes, vk::PresentModeKHR::eImmediate))
	{
		return vk::PresentModeKHR::eImmediate;
	}

	if (std::ranges::contains(availableModes, vk::PresentModeKHR::eMailbox))
	{
		return vk::PresentModeKHR::eMailbox;
	}

	return vk::PresentModeKHR::eFifo;
}

vk::Extent2D FSwapChain::CalculateSwapChainExtent(const vk::SurfaceCapabilitiesKHR& capabilities) const
{
	const glm::ivec2 framebufferSize = gEngine->GetWindow()->GetFrameBufferSize();
	TURBO_CHECK(framebufferSize.x > 0 && framebufferSize.y > 0);

	glm::uvec2 unsignedSize = framebufferSize;
	unsignedSize.x = glm::clamp(unsignedSize.x, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
	unsignedSize.y = glm::clamp(unsignedSize.y, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

	return  {unsignedSize.x, unsignedSize.y};
}

void FSwapChain::InitializeImageViews()
{
	TURBO_LOG(LOG_RHI, Display, "Creating swap chain's image views")

	vk::ImageSubresourceRange subresourceRange{};
	subresourceRange.setAspectMask(vk::ImageAspectFlagBits::eColor);
	subresourceRange.setLayerCount(1);
	subresourceRange.setLevelCount(1);

	vk::ImageViewCreateInfo createInfo{};
	createInfo.setViewType(vk::ImageViewType::e2D);
	createInfo.setFormat(mImageFormat);
	createInfo.setSubresourceRange(subresourceRange);

	for (FSwapchainImageData& imageData : mImageDatas)
	{
		createInfo.setImage(imageData.Image);

		vk::Result result;
		std::tie(result, imageData.ImageView) = mDevice->Get().createImageView(createInfo);
		CHECK_VULKAN_HPP_MSG(result, "Cannot create swapchain image view");
	}
}

void FSwapChain::InitializeSemaphores()
{
	const vk::SemaphoreCreateInfo semaphoreCreateInfo = VulkanInitializers::SemaphoreCreateInfo();
	for (FSwapchainImageData& imageData : mImageDatas)
	{
		CHECK_VULKAN_RESULT(imageData.QueueSubmitSemaphore, mDevice->Get().createSemaphore(semaphoreCreateInfo));
	}
}
