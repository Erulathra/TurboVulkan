#pragma once

#include "RHICore.h"

namespace Turbo
{
	class FVulkanDevice;

	class FSwapChain
	{
	public:

	public:
		void Init(const FVulkanDevice* InDevice);
		void Destroy();

	private:
		VkSurfaceFormatKHR SelectBestSurfacePixelFormat(const std::vector<VkSurfaceFormatKHR>& AvailableFormats) const;
		VkPresentModeKHR SelectBestPresentMode(const std::vector<VkPresentModeKHR>& AvailableModes) const;
		VkExtent2D CalculateSwapChainExtent(const VkSurfaceCapabilitiesKHR& Capabilities) const;

	private:
		void InitializeImageViews(const FVulkanDevice* Device);

	private:
		VkSwapchainKHR mVulkanSwapChain = nullptr;

		VkFormat mImageFormat = VK_FORMAT_UNDEFINED;
		VkExtent2D mImageSize{};

		std::vector<VkImage> mImages;
		std::vector<VkImageView> mImageViews;
	};
}
