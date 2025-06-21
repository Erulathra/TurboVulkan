#include "Core/RHI/VulkanDevice.h"

#include "Core/Engine.h"
#include "Core/RHI/VulkanHardwareDevice.h"
#include "Core/RHI/Utils/VulkanInitializers.h"
#include "Core/RHI/VulkanRHI.h"

using namespace Turbo;

bool FVulkanDevice::AcquiredQueues::IsValid() const
{
    return GraphicsQueue
        && PresentQueue
        && TransferQueue;
}

FVulkanDevice::FVulkanDevice(FVulkanHardwareDevice& hardwareDevice)
    : mHardwareDevice(&hardwareDevice)
{
}

void FVulkanDevice::Init()
{
    TURBO_CHECK(mHardwareDevice->IsValid());

    vk::Instance vkInstance = gEngine->GetRHI()->GetVulkanInstance();
    TURBO_CHECK(vkInstance);

    TURBO_LOG(LOG_RHI, LOG_INFO, "Creating logical device.")

    mQueueIndices = mHardwareDevice->GetQueueFamilyIndices();
    if (!mQueueIndices.IsValid())
    {
        TURBO_LOG(LOG_RHI, LOG_ERROR, "Cannot obtain queue family indices.")
        return;
    }

    const std::set<uint32> uniqueQueues = mQueueIndices.GetUniqueQueueIndices();
    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
    queueCreateInfos.reserve(uniqueQueues.size());

    for (uint32 queueFamilyIndex : uniqueQueues)
    {
        constexpr std::array queuePriorities = {1.f};

        vk::DeviceQueueCreateInfo QueueCreateInfo{};
        QueueCreateInfo.setQueuePriorities(queuePriorities);
        QueueCreateInfo.setQueueFamilyIndex(queueFamilyIndex);

        queueCreateInfos.push_back(QueueCreateInfo);
    }

    vk::DeviceCreateInfo deviceCreateInfo{};
    deviceCreateInfo.setQueueCreateInfos(queueCreateInfos);
    deviceCreateInfo.setPEnabledExtensionNames(RequiredDeviceExtensions);

    vk::PhysicalDeviceFeatures deviceFeatures = GetRequiredDeviceFeatures();
    deviceCreateInfo.setPEnabledFeatures(&deviceFeatures);

    vk::PhysicalDeviceSynchronization2Features synchronizationFeatures{vk::True};
    vk::PhysicalDeviceDynamicRenderingFeatures dynamicRenderingFeatures{vk::True};
    synchronizationFeatures.setPNext(&dynamicRenderingFeatures);
    deviceCreateInfo.setPNext(&synchronizationFeatures);

    vk::Result vkResult;
    std::tie(vkResult, mVulkanDevice) = mHardwareDevice->Get().createDevice(deviceCreateInfo);
    CHECK_VULKAN_HPP_MSG(vkResult, "Cannot create logical device");

    VULKAN_HPP_DEFAULT_DISPATCHER.init(mVulkanDevice);

    SetupQueues();
    SetupCommandPools();
}

void FVulkanDevice::SetupQueues()
{
    TURBO_CHECK(mVulkanDevice && mQueueIndices.IsValid());

    if (mQueueIndices.IsValid())
    {
        mQueues.GraphicsQueue = mVulkanDevice.getQueue(mQueueIndices.GraphicsFamily, 0);
        mQueues.PresentQueue = mVulkanDevice.getQueue(mQueueIndices.PresentFamily, 0);
        mQueues.TransferQueue = mVulkanDevice.getQueue(mQueueIndices.TransferFamily, 0);
    }

    TURBO_CHECK_MSG(mQueues.IsValid(), "Cannot obtain required device queues.")
}

void FVulkanDevice::Destroy()
{
    TURBO_LOG(LOG_RHI, LOG_INFO, "Destroying logical device.");

    if (mRenderCommandPool)
    {
        mVulkanDevice.destroyCommandPool(mRenderCommandPool);
    }

    mVulkanDevice.destroy();
    mVulkanDevice = nullptr;
}

bool FVulkanDevice::IsValid() const
{
    return mVulkanDevice != nullptr
        && mQueueIndices.IsValid()
        && mRenderCommandPool != nullptr;
}

vk::PhysicalDeviceFeatures FVulkanDevice::GetRequiredDeviceFeatures()
{
    vk::PhysicalDeviceFeatures supportedFeatures = mHardwareDevice->GetSupportedFeatures();

    vk::PhysicalDeviceFeatures features{};
    features.textureCompressionBC = true;
    features.fillModeNonSolid = supportedFeatures.fillModeNonSolid;
    features.wideLines = supportedFeatures.wideLines;
    features.samplerAnisotropy = supportedFeatures.samplerAnisotropy;
    features.sampleRateShading = supportedFeatures.sampleRateShading;

    return features;
}

void FVulkanDevice::SetupCommandPools()
{
    TURBO_LOG(LOG_RHI, LOG_INFO, "Creating command pools.");

    const vk::CommandPoolCreateInfo createInfo = VulkanInitializers::CommandPoolCreateInfo(
        mHardwareDevice->GetQueueFamilyIndices().GraphicsFamily,
        vk::CommandPoolCreateFlagBits::eResetCommandBuffer
    );

    vk::Result result;
    std::tie(result, mRenderCommandPool) = mVulkanDevice.createCommandPool(createInfo);
    CHECK_VULKAN_HPP(result);
}

