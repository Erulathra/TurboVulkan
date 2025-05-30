#pragma once

#include "RHICore.h"

#include "QueueFamilyIndices.h"
#include "Core/Window.h"

namespace Turbo
{
	struct SwapChainDeviceSupportDetails
	{
		VkSurfaceCapabilitiesKHR Capabilities;
		std::vector<VkSurfaceFormatKHR> Formats;
		std::vector<VkPresentModeKHR> PresentModes;
	};

	const static std::vector<const char*> RequiredDeviceExtensions =
	{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	class FVulkanHardwareDevice
	{
	public:
		FVulkanHardwareDevice() = delete;
		explicit FVulkanHardwareDevice(VkPhysicalDevice PhysicalDevice)
			: mVulkanPhysicalDevice(PhysicalDevice)
		{
			TURBO_CHECK(PhysicalDevice);
		}

		[[nodiscard]] bool IsValid() const { return mVulkanPhysicalDevice; }

	public:
		[[nodiscard]] int32 CalculateDeviceScore() const;
		[[nodiscard]] bool AreExtensionsSupportedByDevice(const std::vector<const char*>& RequiredExtensions) const;
		[[nodiscard]] FQueueFamilyIndices FindQueueFamilies() const;
		[[nodiscard]] bool IsDeviceCapable() const;
		[[nodiscard]] SwapChainDeviceSupportDetails QuerySwapChainSupport() const;

	public:
		[[nodiscard]] VkPhysicalDevice GetVulkanPhysicalDevice() const { return mVulkanPhysicalDevice; }

	private:
		VkPhysicalDevice mVulkanPhysicalDevice = nullptr;
	};
}
