#include "Core/RHI/SwapChain.h"

#include "Core/Engine.h"
#include "Core/Window.h"
#include "Core/RHI/HardwareDevice.h"
#include "Core/RHI/Device.h"
#include "Core/RHI/VulkanRHI.h"

using namespace Turbo;

void SwapChain::Init(const Device* InDevice)
{
	TURBO_CHECK(IsValid(InDevice));

	const HardwareDevice* HWDevice = gEngine->GetRHI()->GetHardwareDevice();

	TURBO_LOG(LOG_RHI, LOG_INFO, "Creating Swap chain");

	const SwapChainDeviceSupportDetails SupportDetails = HWDevice->QuerySwapChainSupport();

	uint32 ImageCount = SupportDetails.Capabilities.minImageCount + 1;
	if (SupportDetails.Capabilities.maxImageCount > 0)
	{
		ImageCount = glm::min(ImageCount, SupportDetails.Capabilities.maxImageCount);
	}

	VkSwapchainCreateInfoKHR CreateInfo{};
	CreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	CreateInfo.surface = gEngine->GetWindow()->GetVulkanSurface();

	CreateInfo.minImageCount = ImageCount;
	VkSurfaceFormatKHR PixelFormat = SelectBestSurfacePixelFormat(SupportDetails.Formats);
	CreateInfo.imageFormat = PixelFormat.format;
	CreateInfo.imageColorSpace = PixelFormat.colorSpace;
	CreateInfo.imageExtent = CalculateSwapChainExtent(SupportDetails.Capabilities);
	CreateInfo.imageArrayLayers = 1;
	CreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	const QueueFamilyIndices& QueueIndices = InDevice->GetQueueIndices();
	const std::set<uint32> UniqueQueueIndices = QueueIndices.GetUniqueQueueIndices();
	const std::vector<uint32> QueueIndicesVector(UniqueQueueIndices.begin(), UniqueQueueIndices.end());
	if (QueueIndices.GraphicsFamily == QueueIndices.PresentFamily)
	{
		CreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}
	else
	{
		CreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		CreateInfo.queueFamilyIndexCount = QueueIndices.Num();
		CreateInfo.pQueueFamilyIndices = QueueIndicesVector.data();
	}

	CreateInfo.preTransform = SupportDetails.Capabilities.currentTransform;
	CreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

	CreateInfo.presentMode = SelectBestPresentMode(SupportDetails.PresentModes);
	CreateInfo.clipped = VK_TRUE;

	CreateInfo.oldSwapchain = nullptr;

	const VkResult SwapChaindCreationResult = vkCreateSwapchainKHR(InDevice->GetVulkanDevice(), &CreateInfo, nullptr, &VulkanSwapChain);
	CHECK_VULKAN_RESULT(SwapChaindCreationResult, "Error: {} during creating swap chain.");

	ImageFormat = CreateInfo.imageFormat;
	ImageSize = CreateInfo.imageExtent;

	uint32 SwapChainImagesNum;
	vkGetSwapchainImagesKHR(InDevice->GetVulkanDevice(), VulkanSwapChain, &SwapChainImagesNum, nullptr);
	Images.resize(SwapChainImagesNum);
	vkGetSwapchainImagesKHR(InDevice->GetVulkanDevice(), VulkanSwapChain, &ImageCount, Images.data());

	InitializeImageViews(InDevice);
}


void SwapChain::Destroy()
{
	TURBO_LOG(LOG_RHI, LOG_INFO, "Destroying Swap chain");

	const Device* Device = gEngine->GetRHI()->GetDevice();
	TURBO_CHECK(Device);

	VkDevice VulkanDevice = Device->GetVulkanDevice();
	vkDestroySwapchainKHR(VulkanDevice, VulkanSwapChain, nullptr);

	for (VkImageView& ImageView : ImageViews)
	{
		vkDestroyImageView(VulkanDevice, ImageView, nullptr);
	}
	ImageViews.clear();
}

VkSurfaceFormatKHR SwapChain::SelectBestSurfacePixelFormat(const std::vector<VkSurfaceFormatKHR>& AvailableFormats) const
{
	TURBO_CHECK(!AvailableFormats.empty());

	for (const VkSurfaceFormatKHR& Format : AvailableFormats)
	{
		if (Format.format == VK_FORMAT_B8G8R8A8_SRGB
			&& Format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			return Format;
		}
	}

	return AvailableFormats.front();
}

VkPresentModeKHR SwapChain::SelectBestPresentMode(const std::vector<VkPresentModeKHR>& AvailableModes) const
{
	TURBO_CHECK(!AvailableModes.empty());

	if (std::ranges::contains(AvailableModes, VK_PRESENT_MODE_IMMEDIATE_KHR))
	{
		return VK_PRESENT_MODE_IMMEDIATE_KHR;
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D SwapChain::CalculateSwapChainExtent(const VkSurfaceCapabilitiesKHR& Capabilities) const
{
	glm::uvec2 FramebufferSize = gEngine->GetWindow()->GetFrameBufferSize();
	FramebufferSize.x = glm::clamp(FramebufferSize.x, Capabilities.minImageExtent.width, Capabilities.maxImageExtent.width);
	FramebufferSize.y = glm::clamp(FramebufferSize.y, Capabilities.minImageExtent.height, Capabilities.maxImageExtent.height);

	return  {FramebufferSize.x, FramebufferSize.y};
}

void SwapChain::InitializeImageViews(const Device* Device)
{
	TURBO_LOG(LOG_RHI, LOG_DISPLAY, "Creating swap chain's image views")

	ImageViews.resize(Images.size());

	for (uint32 ImageId = 0; ImageId < ImageViews.size(); ++ImageId)
	{
		VkImageViewCreateInfo CreateInfo;
		CreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		CreateInfo.image = Images[ImageId];
		CreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		CreateInfo.format = ImageFormat;

		CreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		CreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		CreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		CreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		CreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		CreateInfo.subresourceRange.baseMipLevel = 0;
		CreateInfo.subresourceRange.levelCount = 1;
		CreateInfo.subresourceRange.baseArrayLayer = 0;
		CreateInfo.subresourceRange.layerCount = 1;

		const VkResult CreateImageViewResult = vkCreateImageView(Device->GetVulkanDevice(), &CreateInfo, nullptr, &ImageViews[ImageId]);
		CHECK_VULKAN_RESULT(CreateImageViewResult, "Error: {} during creation swap chain's image view");
	}
}
