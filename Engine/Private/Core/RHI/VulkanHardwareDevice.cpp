#include "Core/RHI/VulkanHardwareDevice.h"

#include "Core/Engine.h"
#include "Core/Window.h"
#include "Core/RHI/VulkanRHI.h"

using namespace Turbo;


FVulkanHardwareDevice::FVulkanHardwareDevice(const vk::PhysicalDevice& physicalDevice): mVulkanPhysicalDevice(physicalDevice)
{
	mQueueFamilyIndices = FindQueueFamilies();
	mSupportedFeatures = physicalDevice.getFeatures();
	mProperties = physicalDevice.getProperties();
}

int32 FVulkanHardwareDevice::CalculateDeviceScore() const
{
	int32 deviceScore = 0;
	deviceScore += -1024 * (mProperties.deviceType == vk::PhysicalDeviceType::eVirtualGpu);
	deviceScore += -1024 * (mProperties.deviceType == vk::PhysicalDeviceType::eCpu);
	deviceScore += 1024 * (mProperties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu);

	deviceScore += 128 * (mQueueFamilyIndices.PresentFamily == mQueueFamilyIndices.GraphicsFamily);
	deviceScore += 128 * (mQueueFamilyIndices.GraphicsFamily == mQueueFamilyIndices.TransferFamily);

	return deviceScore;
}

bool FVulkanHardwareDevice::IsDeviceCapable() const
{
	bool bResult = true;

	bResult &= mProperties.apiVersion >= VULKAN_VERSION;
	bResult &= mQueueFamilyIndices.IsValid();
	bResult &= AreExtensionsSupportedByDevice(RequiredDeviceExtensions);

	if (bResult)
	{
		const SwapChainDeviceSupportDetails SwapChainSupportDetails = QuerySwapChainSupport();
		bResult &= !SwapChainSupportDetails.Formats.empty();
		bResult &= !SwapChainSupportDetails.PresentModes.empty();
	}

	return bResult;
}

bool FVulkanHardwareDevice::AreExtensionsSupportedByDevice(const std::vector<const char*>& requiredExtensions) const
{
	vk::Result vkResult;
	std::vector<vk::ExtensionProperties> extensionProperties;
	std::tie(vkResult, extensionProperties) = mVulkanPhysicalDevice.enumerateDeviceExtensionProperties();

	if (vkResult == vk::Result::eSuccess)
	{
		std::set<std::string> requiredExtensionsSet(requiredExtensions.begin(), requiredExtensions.end());
		for (const vk::ExtensionProperties& extension : extensionProperties)
		{
			requiredExtensionsSet.erase(std::string(extension.extensionName));
		}

		return requiredExtensionsSet.empty();
	}

	return false;
}

FQueueFamilyIndices FVulkanHardwareDevice::FindQueueFamilies() const
{
	FQueueFamilyIndices result{};

	FWindow* window = gEngine->GetWindow();
	TURBO_CHECK(window);

	const std::vector<vk::QueueFamilyProperties> queueFamilyProperties = mVulkanPhysicalDevice.getQueueFamilyProperties();
	for (int32 queueId = 0; queueId < queueFamilyProperties.size(); ++queueId)
	{
		const vk::QueueFamilyProperties& familyProperties = queueFamilyProperties[queueId];
		if (familyProperties.queueFlags & vk::QueueFlagBits::eGraphics)
		{
			result.GraphicsFamily = queueId;
		}

		if (familyProperties.queueFlags & vk::QueueFlagBits::eTransfer)
		{
			result.TransferFamily = queueId;
		}

		vk::Result vkResult;
		vk::Bool32 bPresentSupport = false;
		std::tie(vkResult, bPresentSupport) = mVulkanPhysicalDevice.getSurfaceSupportKHR(queueId, window->GetVulkanSurface());
		CHECK_VULKAN_HPP(vkResult);

		if (bPresentSupport == true)
		{
			result.PresentFamily = queueId;
		}
	}

	return result;
}

SwapChainDeviceSupportDetails FVulkanHardwareDevice::QuerySwapChainSupport() const
{
	const vk::SurfaceKHR surface = gEngine->GetWindow()->GetVulkanSurface();
	TURBO_CHECK(surface);

	vk::Result vkResult;
	SwapChainDeviceSupportDetails result;
	std::tie(vkResult, result.Capabilities) = mVulkanPhysicalDevice.getSurfaceCapabilitiesKHR(surface);
	std::tie(vkResult, result.Formats) = mVulkanPhysicalDevice.getSurfaceFormatsKHR(surface);
	std::tie(vkResult, result.PresentModes) = mVulkanPhysicalDevice.getSurfacePresentModesKHR(surface);

	return result;
}
