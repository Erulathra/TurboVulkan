#include "Core/RHI/LogicalDevice.h"

#include "Core/Engine.h"
#include "Core/RHI/HardwareDevice.h"
#include "Core/RHI/VulkanRHI.h"

using namespace Turbo;

bool LogicalDevice::AcquiredQueues::IsValid() const
{
    return GraphicsQueue
        && PresentQueue;
}

void LogicalDevice::Init(const HardwareDevicePtr& InHWDevice)
{
    VkInstance VulkanInstance = VulkanRHI::Get()->GetVulkanInstance();
    TURBO_CHECK(InHWDevice && VulkanInstance);

    HWDevice = InHWDevice;


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
        Engine::Get()->RequestExit(EExitCode::RHICriticalError);
        return;
    }

    // TODO: For now We assume that we would have only one device.
    volkLoadDevice(VulkanDevice);

    SetupQueues();
}

void LogicalDevice::SetupQueues()
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
        Engine::Get()->RequestExit(EExitCode::RHICriticalError);
        return;
    }
}

void LogicalDevice::Destroy()
{
    TURBO_LOG(LOG_RHI, LOG_INFO, "Destroying logical device.");
    vkDestroyDevice(VulkanDevice, nullptr);
    VulkanDevice = nullptr;
}

bool LogicalDevice::IsValid() const
{
    return VulkanDevice != nullptr
        && IsValid<HardwareDevice>(HWDevice)
        && QueueIndices.IsValid();
}

VkPhysicalDeviceFeatures LogicalDevice::GetRequiredDeviceFeatures()
{
    VkPhysicalDeviceFeatures Result{};
    Result.textureCompressionBC = true;

    return Result;
}

