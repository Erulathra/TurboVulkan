#include "Core/RHI/HardwareDevice.h"

#include "Core/Window.h"
#include "Core/RHI/VulkanRHI.h"

using namespace Turbo;


int32 HardwareDevice::CalculateDeviceScore() const
{
	VkPhysicalDeviceProperties DeviceProperties;
	VkPhysicalDeviceFeatures DeviceFeatures;
	VkPhysicalDeviceMemoryProperties MemoryProperties;

	vkGetPhysicalDeviceProperties(VulkanPhysicalDevice, &DeviceProperties);
	vkGetPhysicalDeviceFeatures(VulkanPhysicalDevice, &DeviceFeatures);
	vkGetPhysicalDeviceMemoryProperties(VulkanPhysicalDevice, &MemoryProperties);


	int32 DeviceScore = 0;
	DeviceScore += -1024 * (DeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU);
	DeviceScore += -1024 * (DeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_CPU);
	DeviceScore += 1024 * (DeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU);

	QueueFamilyIndices QueueIndices = FindQueueFamilies();
	DeviceScore += 128 * (QueueIndices.PresentFamily == QueueIndices.GraphicsFamily);

	return DeviceScore;
}

bool HardwareDevice::IsDeviceCapable() const
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

bool HardwareDevice::AreExtensionsSupportedByDevice(const std::vector<const char*>& RequiredExtensions) const
{
	uint32 DeviceExtensionNum;
	vkEnumerateDeviceExtensionProperties(VulkanPhysicalDevice, nullptr, &DeviceExtensionNum, nullptr);

	std::vector<VkExtensionProperties> DeviceExtensionProperties(DeviceExtensionNum);
	vkEnumerateDeviceExtensionProperties(VulkanPhysicalDevice, nullptr, &DeviceExtensionNum, DeviceExtensionProperties.data());

	std::set<std::string> RequiredExtensionsSet(RequiredExtensions.begin(), RequiredExtensions.end());
	for (auto Extension : DeviceExtensionProperties)
	{
		RequiredExtensionsSet.erase(std::string(Extension.extensionName));
	}

	return RequiredExtensionsSet.empty();
}

QueueFamilyIndices HardwareDevice::FindQueueFamilies() const
{
	QueueFamilyIndices Result{};

	Window* MainWindow = Window::GetMain();
	TURBO_CHECK(MainWindow);

	uint32 QueueFamilyNum;
	vkGetPhysicalDeviceQueueFamilyProperties(VulkanPhysicalDevice, &QueueFamilyNum, nullptr);

	std::vector<VkQueueFamilyProperties> QueueFamilyProperties(QueueFamilyNum);
	vkGetPhysicalDeviceQueueFamilyProperties(VulkanPhysicalDevice, &QueueFamilyNum, QueueFamilyProperties.data());

	for (int32 QueueId = 0; QueueId < QueueFamilyNum; ++QueueId)
	{
		const VkQueueFamilyProperties& FamilyProperties = QueueFamilyProperties[QueueId];
		if (FamilyProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			Result.GraphicsFamily = QueueId;
		}

		VkBool32 bPresentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(VulkanPhysicalDevice, QueueId, MainWindow->GetVulkanSurface(), &bPresentSupport);
		if (bPresentSupport == true)
		{
			Result.PresentFamily = QueueId;
		}
	}

	return Result;
}

SwapChainDeviceSupportDetails HardwareDevice::QuerySwapChainSupport() const
{
	VkSurfaceKHR Surface = Window::GetMain()->GetVulkanSurface();
	TURBO_CHECK(Surface);

	SwapChainDeviceSupportDetails Result;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(VulkanPhysicalDevice, Surface, &Result.Capabilities);

	uint32 SurfaceFormatNum;
	vkGetPhysicalDeviceSurfaceFormatsKHR(VulkanPhysicalDevice, Surface, &SurfaceFormatNum, nullptr);

	if (SurfaceFormatNum > 0)
	{
		Result.Formats.resize(SurfaceFormatNum);
		vkGetPhysicalDeviceSurfaceFormatsKHR(VulkanPhysicalDevice, Surface, &SurfaceFormatNum, Result.Formats.data());
	}

	uint32 PresentModesNum;
	vkGetPhysicalDeviceSurfacePresentModesKHR(VulkanPhysicalDevice, Surface, &PresentModesNum, nullptr);

	if (PresentModesNum > 0)
	{
		Result.PresentModes.resize(PresentModesNum);
		vkGetPhysicalDeviceSurfacePresentModesKHR(VulkanPhysicalDevice, Surface, &PresentModesNum, Result.PresentModes.data());
	}

	return Result;
}
