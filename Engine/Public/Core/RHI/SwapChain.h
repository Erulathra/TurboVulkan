#pragma once

#include "RHICore.h"

namespace Turbo
{
	class HardwareDevice;

	class SwapChain
	{
	public:

	public:
		void Init(const LogicalDevicePtr& InDevice);
		void Destroy();

	private:
		VkSurfaceFormatKHR SelectBestSurfacePixelFormat(const std::vector<VkSurfaceFormatKHR>& AvailableFormats) const;
		VkPresentModeKHR SelectBestPresentMode(const std::vector<VkPresentModeKHR>& AvailableModes) const;
		VkExtent2D CalculateSwapChainExtent(const VkSurfaceCapabilitiesKHR& Capabilities) const;

	private:
		void InitializeImageViews(LogicalDevice* Device);

	private:
		LogicalDeviceWeakPtr Device;

		VkSwapchainKHR VulkanSwapChain = nullptr;

		VkFormat ImageFormat = VK_FORMAT_UNDEFINED;
		VkExtent2D ImageSize{};

		std::vector<VkImage> Images;
		std::vector<VkImageView> ImageViews;
	};
}
