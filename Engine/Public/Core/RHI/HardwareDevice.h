#pragma once
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

	class HardwareDevice
	{

	public:
		HardwareDevice() = delete;
		explicit HardwareDevice(VkPhysicalDevice PhysicalDevice)
			: VulkanPhysicalDevice(PhysicalDevice)
		{
			TURBO_CHECK(PhysicalDevice);
		}

		[[nodiscard]] bool IsValid() const { return VulkanPhysicalDevice; }

	public:
		[[nodiscard]] int32 CalculateDeviceScore() const;
		[[nodiscard]] bool AreExtensionsSupportedByDevice(const std::vector<const char*>& RequiredExtensions) const;
		[[nodiscard]] QueueFamilyIndices FindQueueFamilies() const;
		[[nodiscard]] bool IsDeviceCapable() const;
		[[nodiscard]] SwapChainDeviceSupportDetails QuerySwapChainSupport() const;

	public:
		[[nodiscard]] VkPhysicalDevice GetVulkanPhysicalDevice() const { return VulkanPhysicalDevice; }

	private:
		VkPhysicalDevice VulkanPhysicalDevice = nullptr;
	};
}
