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

		VkSurfaceFormatKHR SelectBestSurfacePixelFormat(const std::vector<VkSurfaceFormatKHR>& AvailableFormats) const;
		VkPresentModeKHR SelectBestPresentMode(const std::vector<VkPresentModeKHR>& AvailableModes) const;
		VkExtent2D CalculateSwapChainExtent(const VkSurfaceCapabilitiesKHR& Capabilities) const;

	private:
		LogicalDeviceWeakPtr Device;

		VkSwapchainKHR VulkanSwapChain = nullptr;

		std::vector<VkImage> Images;
		VkFormat ImageFormat = VK_FORMAT_UNDEFINED;
		VkExtent2D ImageSize{};
	};
}
