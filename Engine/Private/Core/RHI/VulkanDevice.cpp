#include "Core/RHI/VulkanDevice.h"

#include "Core/Engine.h"
#include "Core/RHI/VulkanHardwareDevice.h"
#include "Core/RHI/VulkanRHI.h"

using namespace Turbo;

bool FVulkanDevice::AcquiredQueues::IsValid() const
{
    return GraphicsQueue
        && PresentQueue;
}

void FVulkanDevice::Init(const FVulkanHardwareDevice* InHWDevice)
{
    VkInstance VulkanInstance = gEngine->GetRHI()->GetVulkanInstance();
    TURBO_CHECK(InHWDevice && VulkanInstance);

    TURBO_LOG(LOG_RHI, LOG_INFO, "Creating logical device.")

    mQueueIndices = InHWDevice->FindQueueFamilies();
    if (!mQueueIndices.IsValid())
    {
        TURBO_LOG(LOG_RHI, LOG_ERROR, "Cannot obtain queue family indices.")
        return;
    }

    const std::set<uint32> UniqueQueues = mQueueIndices.GetUniqueQueueIndices();
    std::vector<VkDeviceQueueCreateInfo> QueueCreateInfos;
    QueueCreateInfos.reserve(UniqueQueues.size());

    constexpr float QueuePriority = 1.f;

    for (uint32 QueueFamily : UniqueQueues)
    {
        VkDeviceQueueCreateInfo QueueCreateInfo{};
        QueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        QueueCreateInfo.queueFamilyIndex = QueueFamily;
        QueueCreateInfo.queueCount = 1;
        QueueCreateInfo.pQueuePriorities = &QueuePriority;

        QueueCreateInfos.push_back(QueueCreateInfo);
    }

    VkDeviceCreateInfo DeviceCreateInfo{};
    DeviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    DeviceCreateInfo.queueCreateInfoCount = QueueCreateInfos.size();
    DeviceCreateInfo.pQueueCreateInfos = QueueCreateInfos.data();;
    DeviceCreateInfo.enabledExtensionCount = RequiredDeviceExtensions.size();
    DeviceCreateInfo.ppEnabledExtensionNames = RequiredDeviceExtensions.data();

    VkPhysicalDeviceFeatures DeviceFeatures = GetRequiredDeviceFeatures();
    DeviceCreateInfo.pEnabledFeatures = &DeviceFeatures;

    const VkResult DeviceCreationResult = vkCreateDevice(InHWDevice->GetVulkanPhysicalDevice(), &DeviceCreateInfo, nullptr, &mVulkanDevice);
    if (DeviceCreationResult != VK_SUCCESS)
    {
        TURBO_LOG(LOG_RHI, LOG_ERROR, "Error: {} during creating logical device.", static_cast<int32>(DeviceCreationResult));
        gEngine->RequestExit(EExitCode::RHICriticalError);
        return;
    }

    // TODO: For now We assume that we would have only one device.
    volkLoadDevice(mVulkanDevice);

    SetupQueues();
}

void FVulkanDevice::SetupQueues()
{
    TURBO_CHECK(IsValid());

    if (mQueueIndices.IsValid())
    {
        vkGetDeviceQueue(mVulkanDevice, mQueueIndices.GraphicsFamily, 0, &mQueues.GraphicsQueue);
        vkGetDeviceQueue(mVulkanDevice, mQueueIndices.PresentFamily, 0, &mQueues.PresentQueue);
    }

    if (!mQueues.IsValid())
    {
        TURBO_LOG(LOG_RHI, LOG_ERROR, "Required queues not found.")
        gEngine->RequestExit(EExitCode::RHICriticalError);
        return;
    }
}

void FVulkanDevice::Destroy()
{
    TURBO_LOG(LOG_RHI, LOG_INFO, "Destroying logical device.");
    vkDestroyDevice(mVulkanDevice, nullptr);
    mVulkanDevice = nullptr;
}

bool FVulkanDevice::IsValid() const
{
    return mVulkanDevice != nullptr
        && mQueueIndices.IsValid();
}

VkPhysicalDeviceFeatures FVulkanDevice::GetRequiredDeviceFeatures()
{
    VkPhysicalDeviceFeatures Result{};
    Result.textureCompressionBC = true;

    return Result;
}

