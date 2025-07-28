#include "Core/RHI/VulkanDevice.h"

#include "Core/Engine.h"
#include "Core/RHI/Buffer.h"
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

    TURBO_LOG(LOG_RHI, Info, "Creating logical device.")

    mQueueIndices = mHardwareDevice->GetQueueFamilyIndices();
    if (!mQueueIndices.IsValid())
    {
        TURBO_LOG(LOG_RHI, Error, "Cannot obtain queue family indices.")
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

    vk::PhysicalDeviceDynamicRenderingFeatures dynamicRenderingFeatures{true};
    dynamicRenderingFeatures.setPNext(nullptr);

    vk::PhysicalDeviceSynchronization2Features synchronizationFeatures{true};
    synchronizationFeatures.setPNext(dynamicRenderingFeatures);

    vk::PhysicalDeviceBufferDeviceAddressFeatures bufferDeviceAddressFeatures{true};
    bufferDeviceAddressFeatures.setPNext(synchronizationFeatures);

    vk::PhysicalDeviceFeatures2 deviceFeatures = GetRequiredDeviceFeatures();
    deviceFeatures.setPNext(bufferDeviceAddressFeatures);

    deviceCreateInfo.setPNext(deviceFeatures);

    vk::Result vkResult;
    std::tie(vkResult, mVulkanDevice) = mHardwareDevice->Get().createDevice(deviceCreateInfo);
    CHECK_VULKAN_HPP_MSG(vkResult, "Cannot create logical device");

    VULKAN_HPP_DEFAULT_DISPATCHER.init(mVulkanDevice);

    InitAllocator();

    SetupQueues();
    SetupCommandPools();
}

void FVulkanDevice::InitAllocator()
{
    TURBO_CHECK(mVulkanDevice && mHardwareDevice);

    TURBO_LOG(LOG_RHI, Info, "Creating memory allocator")

    vma::AllocatorCreateInfo createInfo {};
    createInfo.instance = gEngine->GetRHI()->GetVulkanInstance();
    createInfo.physicalDevice = mHardwareDevice->Get();
    createInfo.device = mVulkanDevice;
    createInfo.flags = vma::AllocatorCreateFlagBits::eBufferDeviceAddress;

    const vk::detail::DispatchLoaderDynamic dispatcher = VULKAN_HPP_DEFAULT_DISPATCHER;

    vma::VulkanFunctions vulkanFunctions{};
    vulkanFunctions.vkGetInstanceProcAddr = dispatcher.vkGetInstanceProcAddr;
    vulkanFunctions.vkGetDeviceProcAddr = dispatcher.vkGetDeviceProcAddr;

    createInfo.pVulkanFunctions = &vulkanFunctions;

    CHECK_VULKAN_RESULT(mAllocator, vma::createAllocator(createInfo))
}

vk::DeviceAddress FVulkanDevice::GetBufferAddress(const FBuffer* buffer)
{
    vk::BufferDeviceAddressInfo deviceAddressInfo{};
    deviceAddressInfo.buffer = buffer->GetBuffer();

    return mVulkanDevice.getBufferAddress(deviceAddressInfo);
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

    TURBO_CHECK_MSG(mQueues.IsValid(), "Cannot obtain required device queues")
}

void FVulkanDevice::Destroy()
{
    TURBO_LOG(LOG_RHI, Info, "Destroying VMA's allocator");
    vmaDestroyAllocator(mAllocator);

    TURBO_LOG(LOG_RHI, Info, "Destroying logical device");

    if (mRenderCommandPool)
    {
        mVulkanDevice.destroyCommandPool(mRenderCommandPool);
    }

    if (mImmediateCommandPool)
    {
        mVulkanDevice.destroyCommandPool(mImmediateCommandPool);
    }

    mVulkanDevice.destroy();
    mVulkanDevice = nullptr;
}

bool FVulkanDevice::IsValid() const
{
    return mVulkanDevice != nullptr
        && mQueueIndices.IsValid()
        && mRenderCommandPool != nullptr
        && mImmediateCommandPool != nullptr;
}

vk::PhysicalDeviceFeatures2 FVulkanDevice::GetRequiredDeviceFeatures() const
{
    vk::PhysicalDeviceFeatures supportedFeatures = mHardwareDevice->GetSupportedFeatures();

    vk::PhysicalDeviceFeatures features{};
    features.textureCompressionBC = true;
    features.fillModeNonSolid = supportedFeatures.fillModeNonSolid;
    features.wideLines = supportedFeatures.wideLines;
    features.samplerAnisotropy = supportedFeatures.samplerAnisotropy;
    features.sampleRateShading = supportedFeatures.sampleRateShading;

    vk::PhysicalDeviceFeatures2 features2;
    features2.setFeatures(features);

    return features2;
}

void FVulkanDevice::SetupCommandPools()
{
    TURBO_LOG(LOG_RHI, Info, "Creating command pools.");

    const vk::CommandPoolCreateInfo createInfo = VulkanInitializers::CommandPoolCreateInfo(
        mHardwareDevice->GetQueueFamilyIndices().GraphicsFamily,
        vk::CommandPoolCreateFlagBits::eResetCommandBuffer
    );

    CHECK_VULKAN_RESULT(mRenderCommandPool, mVulkanDevice.createCommandPool(createInfo));
    CHECK_VULKAN_RESULT(mImmediateCommandPool, mVulkanDevice.createCommandPool(createInfo));
}

