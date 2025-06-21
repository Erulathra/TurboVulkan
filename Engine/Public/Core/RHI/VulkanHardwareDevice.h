#pragma once

#include "RHICore.h"

#include "QueueFamilyIndices.h"
#include "Core/Window.h"

namespace Turbo
{
	struct SwapChainDeviceSupportDetails
	{
		vk::SurfaceCapabilitiesKHR Capabilities;
		std::vector<vk::SurfaceFormatKHR> Formats;
		std::vector<vk::PresentModeKHR> PresentModes;
	};

	const static std::vector<const char*> RequiredDeviceExtensions =
	{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	class FVulkanHardwareDevice
	{
	public:
		FVulkanHardwareDevice() = delete;
		explicit FVulkanHardwareDevice(const vk::PhysicalDevice& physicalDevice);

		[[nodiscard]] bool IsValid() const { return mVulkanPhysicalDevice; }

	public:
		[[nodiscard]] int32 CalculateDeviceScore() const;
		[[nodiscard]] bool AreExtensionsSupportedByDevice(const std::vector<const char*>& requiredExtensions) const;
		[[nodiscard]] bool IsDeviceCapable() const;
		[[nodiscard]] SwapChainDeviceSupportDetails QuerySwapChainSupport() const;

	public:
		[[nodiscard]] vk::PhysicalDevice Get() const { return mVulkanPhysicalDevice; }
		[[nodiscard]] const FQueueFamilyIndices& GetQueueFamilyIndices() const { return mQueueFamilyIndices; }
		[[nodiscard]] const vk::PhysicalDeviceFeatures& GetSupportedFeatures() const { return mSupportedFeatures; }

	private:
		[[nodiscard]] FQueueFamilyIndices FindQueueFamilies() const;

	private:
		vk::PhysicalDevice mVulkanPhysicalDevice{};
		vk::PhysicalDeviceFeatures mSupportedFeatures{};
		vk::PhysicalDeviceProperties mProperties{};
		FQueueFamilyIndices mQueueFamilyIndices{};
	};
}
