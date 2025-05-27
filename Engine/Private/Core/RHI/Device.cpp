#include "Core/RHI/Device.h"

#include "Core/Engine.h"
#include "Core/RHI/HardwareDevice.h"
#include "Core/RHI/VulkanRHI.h"

using namespace Turbo;

bool Device::AcquiredQueues::IsValid() const
{
    return GraphicsQueue
        && PresentQueue;
}

void Device::Init(const HardwareDevice* InHWDevice)
{
    VkInstance VulkanInstance = gEngine->GetRHI()->GetVulkanInstance();
    TURBO_CHECK(InHWDevice && VulkanInstance);

    TURBO_LOG(LOG_RHI, LOG_INFO, "Creating logical device.")

    QueueIndices = InHWDevice->FindQueueFamilies();
    if (!QueueIndices.IsValid())
    {
        TURBO_LOG(LOG_RHI, LOG_ERROR, "Cannot obtain queue family indices.")
        return;
    }

    const std::set<uint32> UniqueQueues = QueueIndices.GetUniqueQueueIndices();
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

    const VkResult DeviceCreationResult = vkCreateDevice(InHWDevice->GetVulkanPhysicalDevice(), &DeviceCreateInfo, nullptr, &VulkanDevice);
    if (DeviceCreationResult != VK_SUCCESS)
    {
        TURBO_LOG(LOG_RHI, LOG_ERROR, "Error: {} during creating logical device.", static_cast<int32>(DeviceCreationResult));
        gEngine->RequestExit(EExitCode::RHICriticalError);
        return;
    }

    // TODO: For now We assume that we would have only one device.
    volkLoadDevice(VulkanDevice);

    SetupQueues();
}

void Device::SetupQueues()
{
    TURBO_CHECK(IsValid());

    if (QueueIndices.IsValid())
    {
        vkGetDeviceQueue(VulkanDevice, QueueIndices.GraphicsFamily, 0, &Queues.GraphicsQueue);
        vkGetDeviceQueue(VulkanDevice, QueueIndices.PresentFamily, 0, &Queues.PresentQueue);
    }

    if (!Queues.IsValid())
    {
        TURBO_LOG(LOG_RHI, LOG_ERROR, "Required queues not found.")
        gEngine->RequestExit(EExitCode::RHICriticalError);
        return;
    }
}

void Device::Destroy()
{
    TURBO_LOG(LOG_RHI, LOG_INFO, "Destroying logical device.");
    vkDestroyDevice(VulkanDevice, nullptr);
    VulkanDevice = nullptr;
}

bool Device::IsValid() const
{
    return VulkanDevice != nullptr
        && QueueIndices.IsValid();
}

VkPhysicalDeviceFeatures Device::GetRequiredDeviceFeatures()
{
    VkPhysicalDeviceFeatures Result{};
    Result.textureCompressionBC = true;

    return Result;
}

