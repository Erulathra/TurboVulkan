#include "Core/RHI/VulkanHardwareDevice.h"

#include "Core/Engine.h"
#include "Core/Window.h"
#include "Core/RHI/VulkanRHI.h"

using namespace Turbo;


int32 FVulkanHardwareDevice::CalculateDeviceScore() const
{
	VkPhysicalDeviceProperties DeviceProperties;
	VkPhysicalDeviceFeatures DeviceFeatures;
	VkPhysicalDeviceMemoryProperties MemoryProperties;

	vkGetPhysicalDeviceProperties(mVulkanPhysicalDevice, &DeviceProperties);
	vkGetPhysicalDeviceFeatures(mVulkanPhysicalDevice, &DeviceFeatures);
	vkGetPhysicalDeviceMemoryProperties(mVulkanPhysicalDevice, &MemoryProperties);


	int32 DeviceScore = 0;
	DeviceScore += -1024 * (DeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU);
	DeviceScore += -1024 * (DeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_CPU);
	DeviceScore += 1024 * (DeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU);

	FQueueFamilyIndices QueueIndices = FindQueueFamilies();
	DeviceScore += 128 * (QueueIndices.PresentFamily == QueueIndices.GraphicsFamily);

	return DeviceScore;
}

bool FVulkanHardwareDevice::IsDeviceCapable() const
{
	bool bResult = true;
	bResult &= FindQueueFamilies().IsValid();
	bResult &= AreExtensionsSupportedByDevice(RequiredDeviceExtensions);

	if (bResult)
	{
		const SwapChainDeviceSupportDetails SwapChainSupportDetails = QuerySwapChainSupport();
		bResult &= !SwapChainSupportDetails.Formats.empty();
		bResult &= !SwapChainSupportDetails.PresentModes.empty();
	}

	return bResult;
}

bool FVulkanHardwareDevice::AreExtensionsSupportedByDevice(const std::vector<const char*>& RequiredExtensions) const
{
	uint32 DeviceExtensionNum;
	vkEnumerateDeviceExtensionProperties(mVulkanPhysicalDevice, nullptr, &DeviceExtensionNum, nullptr);

	std::vector<VkExtensionProperties> DeviceExtensionProperties(DeviceExtensionNum);
	vkEnumerateDeviceExtensionProperties(mVulkanPhysicalDevice, nullptr, &DeviceExtensionNum, DeviceExtensionProperties.data());

	std::set<std::string> RequiredExtensionsSet(RequiredExtensions.begin(), RequiredExtensions.end());
	for (auto Extension : DeviceExtensionProperties)
	{
		RequiredExtensionsSet.erase(std::string(Extension.extensionName));
	}

	return RequiredExtensionsSet.empty();
}

FQueueFamilyIndices FVulkanHardwareDevice::FindQueueFamilies() const
{
	FQueueFamilyIndices Result{};

	FSDLWindow* Window = gEngine->GetWindow();
	TURBO_CHECK(Window);

	uint32 QueueFamilyNum;
	vkGetPhysicalDeviceQueueFamilyProperties(mVulkanPhysicalDevice, &QueueFamilyNum, nullptr);

	std::vector<VkQueueFamilyProperties> QueueFamilyProperties(QueueFamilyNum);
	vkGetPhysicalDeviceQueueFamilyProperties(mVulkanPhysicalDevice, &QueueFamilyNum, QueueFamilyProperties.data());

	for (int32 QueueId = 0; QueueId < QueueFamilyNum; ++QueueId)
	{
		const VkQueueFamilyProperties& FamilyProperties = QueueFamilyProperties[QueueId];
		if (FamilyProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			Result.GraphicsFamily = QueueId;
		}

		VkBool32 bPresentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(mVulkanPhysicalDevice, QueueId, Window->GetVulkanSurface(), &bPresentSupport);
		if (bPresentSupport == true)
		{
			Result.PresentFamily = QueueId;
		}
	}

	return Result;
}

SwapChainDeviceSupportDetails FVulkanHardwareDevice::QuerySwapChainSupport() const
{
	const VkSurfaceKHR Surface = gEngine->GetWindow()->GetVulkanSurface();
	TURBO_CHECK(Surface);

	SwapChainDeviceSupportDetails Result;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(mVulkanPhysicalDevice, Surface, &Result.Capabilities);

	uint32 SurfaceFormatNum;
	vkGetPhysicalDeviceSurfaceFormatsKHR(mVulkanPhysicalDevice, Surface, &SurfaceFormatNum, nullptr);

	if (SurfaceFormatNum > 0)
	{
		Result.Formats.resize(SurfaceFormatNum);
		vkGetPhysicalDeviceSurfaceFormatsKHR(mVulkanPhysicalDevice, Surface, &SurfaceFormatNum, Result.Formats.data());
	}

	uint32 PresentModesNum;
	vkGetPhysicalDeviceSurfacePresentModesKHR(mVulkanPhysicalDevice, Surface, &PresentModesNum, nullptr);

	if (PresentModesNum > 0)
	{
		Result.PresentModes.resize(PresentModesNum);
		vkGetPhysicalDeviceSurfacePresentModesKHR(mVulkanPhysicalDevice, Surface, &PresentModesNum, Result.PresentModes.data());
	}

	return Result;
}
