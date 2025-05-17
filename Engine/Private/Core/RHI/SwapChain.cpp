#include "Core/RHI/SwapChain.h"

#include "Core/Engine.h"
#include "Core/Window.h"
#include "Core/RHI/HardwareDevice.h"
#include "Core/RHI/LogicalDevice.h"

using namespace Turbo;

void SwapChain::Init(const LogicalDevicePtr& InDevice)
{
	TURBO_CHECK(IsValid(InDevice));

	Device = InDevice;
	const HardwareDevice* HWDevice = InDevice->GetHardwareDevice();

	TURBO_LOG(LOG_RHI, LOG_INFO, "Creating Swap chain");

	const SwapChainDeviceSupportDetails SupportDetails = HWDevice->QuerySwapChainSupport();

	uint32 ImageCount = SupportDetails.Capabilities.minImageCount + 1;
	if (SupportDetails.Capabilities.maxImageCount > 0)
	{
		ImageCount = glm::min(ImageCount, SupportDetails.Capabilities.maxImageCount);
	}

	VkSwapchainCreateInfoKHR CreateInfo{};
	CreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	CreateInfo.surface = Window::GetMain()->GetVulkanSurface();

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
	if (SwapChaindCreationResult != VK_SUCCESS)
	{
		TURBO_LOG(LOG_RHI, LOG_ERROR, "Error: {} during creating swap chain.", static_cast<int32>(SwapChaindCreationResult));
		Engine::Get()->RequestExit(EExitCode::RHICriticalError);
		return;
	}

	ImageFormat = CreateInfo.imageFormat;
	ImageSize = CreateInfo.imageExtent;

	uint32 SwapChainImagesNum;
	vkGetSwapchainImagesKHR(InDevice->GetVulkanDevice(), VulkanSwapChain, &SwapChainImagesNum, nullptr);
	Images.resize(SwapChainImagesNum);
	vkGetSwapchainImagesKHR(InDevice->GetVulkanDevice(), VulkanSwapChain, &ImageCount, Images.data());
}


void SwapChain::Destroy()
{
	TURBO_LOG(LOG_RHI, LOG_INFO, "Destroying Swap chain");

	const LogicalDevicePtr LockedDevice = Device.lock();
	TURBO_CHECK(LockedDevice);
	vkDestroySwapchainKHR(LockedDevice->GetVulkanDevice(), VulkanSwapChain, nullptr);
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
	glm::uvec2 FramebufferSize = Window::GetMain()->GetFrameBufferSize();
	FramebufferSize.x = glm::clamp(FramebufferSize.x, Capabilities.minImageExtent.width, Capabilities.maxImageExtent.width);
	FramebufferSize.y = glm::clamp(FramebufferSize.y, Capabilities.minImageExtent.height, Capabilities.maxImageExtent.height);

	return  {FramebufferSize.x, FramebufferSize.y};
}
