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
		VkSwapchainKHR VulkanSwapChain = nullptr;

		VkFormat ImageFormat = VK_FORMAT_UNDEFINED;
		VkExtent2D ImageSize{};

		std::vector<VkImage> Images;
		std::vector<VkImageView> ImageViews;
	};
}
