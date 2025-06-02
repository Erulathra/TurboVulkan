#include "Core/RHI/VulkanRHI.h"

#include "Core/Engine.h"
#include "Core/Window.h"
#include "Core/RHI/SwapChain.h"
#include "Core/RHI/VulkanDevice.h"
#include "Core/RHI/VulkanHardwareDevice.h"

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

namespace Turbo
{
    FVulkanRHI::FVulkanRHI() = default;
    FVulkanRHI::~FVulkanRHI() = default;

    void FVulkanRHI::Init()
    {
        CreateVulkanInstance();
        gEngine->GetWindow()->CreateVulkanSurface(mVulkanInstance);
        AcquirePhysicalDevice();

        if (IsValid(mHardwareDevice))
        {
            mDevice = std::make_unique<FVulkanDevice>(*mHardwareDevice);
            mDevice->Init();
        }

        if (IsValid(mDevice))
        {
            mSwapChain = std::make_unique<FSwapChain>(*mDevice);
            mSwapChain->Init();
        }
    }

    void FVulkanRHI::InitWindow(FSDLWindow* window)
    {
        window->InitForVulkan();
    }

    void FVulkanRHI::Destroy()
    {
        if (mDevice)
        {
            vk::Result result = mDevice->GetVulkanDevice().waitIdle();
        }

        if (mSwapChain)
        {
            mSwapChain->Destroy();
            mSwapChain.reset();
        }

        if (mDevice)
        {
            mDevice->Destroy();
            mDevice.reset();
        }

        mHardwareDevice.reset();

        gEngine->GetWindow()->DestroyVulkanSurface(mVulkanInstance);
        DestroyVulkanInstance();
    }

    void FVulkanRHI::CreateVulkanInstance()
    {
        TURBO_LOG(LOG_RHI, LOG_INFO, "Initialize VOLK");

        VULKAN_HPP_DEFAULT_DISPATCHER.init();

        vk::Result vulkanResult;
        uint32 instanceVersion;
        std::tie(vulkanResult, instanceVersion)  = vk::enumerateInstanceVersion();
        TURBO_CHECK_MSG(vulkanResult == vk::Result::eSuccess && instanceVersion >= VK_API_VERSION_1_3, "Your device doesn't support Vulkan 1.3");

        vk::ApplicationInfo appInfo{};
        appInfo.pApplicationName = "Turbo Vulkan";
        appInfo.applicationVersion = TURBO_VERSION();
        appInfo.pEngineName = "Turbo Vulkan";
        appInfo.engineVersion = TURBO_VERSION();
        appInfo.apiVersion = VK_API_VERSION_1_3;

        vk::InstanceCreateInfo createInfo{};
        createInfo.pApplicationInfo = &appInfo;

        std::vector<const char*> extensionNames = gEngine->GetWindow()->GetVulkanRequiredExtensions();

#if WITH_VALIDATION_LAYERS
        if (CheckValidationLayersSupport())
        {
            TURBO_LOG(LOG_RHI, LOG_INFO, "Validation layers supported.")

            createInfo.enabledLayerCount = kVulkanValidationLayers.size();
            createInfo.ppEnabledLayerNames = kVulkanValidationLayers.data();

            extensionNames.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }
        else
#endif // else !WITH_VALIDATION_LAYERS
        {
            TURBO_LOG(LOG_RHI, LOG_INFO, "Validation layers are unsupported or disabled.")

            createInfo.enabledLayerCount = 0;
            createInfo.ppEnabledLayerNames = nullptr;
        }

        createInfo.enabledExtensionCount = extensionNames.size();
        createInfo.ppEnabledExtensionNames = extensionNames.data();

        TURBO_LOG(LOG_RHI, LOG_INFO, "Creating VKInstance.")
        CHECK_VULKAN_HPP(vk::createInstance(&createInfo, nullptr, &mVulkanInstance));
        VULKAN_HPP_DEFAULT_DISPATCHER.init(mVulkanInstance);

#if WITH_VALIDATION_LAYERS
        SetupValidationLayersCallbacks();
#endif // WITH_VALIDATION_LAYERS

        std::tie(vulkanResult, mExtensionProperties) = vk::enumerateInstanceExtensionProperties();
        CHECK_VULKAN_HPP(vulkanResult);

        std::stringstream extensionsStream;
        for (const vk::ExtensionProperties& extension : mExtensionProperties)
        {
            extensionsStream << "\t" << extension.extensionName << "\n";
        }
        TURBO_LOG(LOG_RHI, LOG_DISPLAY, "Supported Extensions: \n {}", extensionsStream.str());
    }

    void FVulkanRHI::DestroyVulkanInstance()
    {
        if (mVulkanInstance)
        {
#if WITH_VALIDATION_LAYERS
            DestroyValidationLayersCallbacks();
#endif // WITH_VALIDATION_LAYERS

            TURBO_LOG(LOG_RHI, LOG_INFO, "Destroying VKInstance.")
            mVulkanInstance.destroy();
            mVulkanInstance = nullptr;
        }
    }

#if WITH_VALIDATION_LAYERS

    bool FVulkanRHI::CheckValidationLayersSupport()
    {
        bool bSuccess = true;

        vk::ResultValue<std::vector<vk::LayerProperties>> layerProperties = vk::enumerateInstanceLayerProperties();
        CHECK_VULKAN_HPP(layerProperties.result);

        for (const char* requestedValidationLayer : kVulkanValidationLayers)
        {
            bool bLayerFound = false;
            for (const vk::LayerProperties& layerProperty : layerProperties.value)
            {
                if (std::strcmp(layerProperty.layerName.data(), requestedValidationLayer) == 0)
                {
                    bLayerFound = true;
                    break;
                }
            }

            if (!bLayerFound)
            {
                TURBO_LOG(LOG_RHI, LOG_CRITICAL, "Missing Validation Layer: {}", requestedValidationLayer);
                bSuccess = false;
            }
        }

        if (bSuccess)
        {
            TURBO_LOG(LOG_RHI, LOG_INFO, "All requested validation layers are supported.");
        }

        return bSuccess;
    }

    void FVulkanRHI::SetupValidationLayersCallbacks()
    {
        TURBO_LOG(LOG_RHI, LOG_INFO, "Assigning validation layers callback.");


        vk::DebugUtilsMessengerCreateInfoEXT createInfo{};

        using msgSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT;
        using msgType = vk::DebugUtilsMessageTypeFlagBitsEXT;

        createInfo.messageSeverity = msgSeverity::eVerbose | msgSeverity::eInfo | msgSeverity::eWarning | msgSeverity::eError;
        createInfo.messageType = msgType::eGeneral | msgType::eValidation | msgType::ePerformance;
        createInfo.setPfnUserCallback(&FVulkanRHI::HandleValidationLayerCallback);
        createInfo.pUserData = nullptr;

        CHECK_VULKAN_HPP(mVulkanInstance.createDebugUtilsMessengerEXT(&createInfo, nullptr, &mDebugMessengerHandle));
    }

    void FVulkanRHI::DestroyValidationLayersCallbacks()
    {
        if (mDebugMessengerHandle)
        {
            TURBO_LOG(LOG_RHI, LOG_INFO, "Destroying validation layers callback.");
            mVulkanInstance.destroyDebugUtilsMessengerEXT(mDebugMessengerHandle);
        }
    }

    vk::Bool32 FVulkanRHI::HandleValidationLayerCallback(
        vk::DebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        vk::DebugUtilsMessageTypeFlagsEXT messageType,
        const vk::DebugUtilsMessengerCallbackDataEXT* callbackData,
        void* userData)
    {
        using msgSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT;
        if (messageSeverity & msgSeverity::eVerbose)
        {
            TURBO_LOG(LOG_RHI, LOG_DISPLAY, "{}", callbackData->pMessage)
        }
        else if (messageSeverity & msgSeverity::eInfo)
        {
            // Messages with info severity are very verbose, so I reduced its verbosity to display.
            TURBO_LOG(LOG_RHI, LOG_DISPLAY, "{}", callbackData->pMessage)
        }
        else if (messageSeverity & msgSeverity::eWarning)
        {
            TURBO_LOG(LOG_RHI, LOG_WARN, "{}", callbackData->pMessage)
        }
        else if (messageSeverity & msgSeverity::eError)
        {
            TURBO_LOG(LOG_RHI, LOG_ERROR, "{}", callbackData->pMessage)
            TURBO_DEBUG_BREAK();
        }

        return VK_FALSE;
    }

#endif // WITH_VALIDATION_LAYERS

    void FVulkanRHI::AcquirePhysicalDevice()
    {
        vk::Result vulkanResult;
        std::vector<vk::PhysicalDevice> physicalDevices;
        std::tie(vulkanResult, physicalDevices) = mVulkanInstance.enumeratePhysicalDevices();

        if (!physicalDevices.empty())
        {
            std::vector<FVulkanHardwareDevice*> hardwareDevices;
            for (VkPhysicalDevice physicalDevice : physicalDevices)
            {
                FVulkanHardwareDevice* hardwareDevice = new FVulkanHardwareDevice(physicalDevice);
                if (hardwareDevice->IsValid() && hardwareDevice->IsDeviceCapable())
                {
                    hardwareDevices.push_back(hardwareDevice);
                }
            }

            std::ranges::sort(
                hardwareDevices, std::ranges::greater{},
                [this](const FVulkanHardwareDevice* device)
                {
                    return device->CalculateDeviceScore();
                });

            mHardwareDevice = !hardwareDevices.empty() ? std::unique_ptr<FVulkanHardwareDevice>(hardwareDevices[0]) : nullptr;

            for (int DeviceId = 1; DeviceId < hardwareDevices.size(); ++DeviceId)
            {
                delete hardwareDevices[DeviceId];
                hardwareDevices[DeviceId] = nullptr;
            }
        }

        TURBO_CHECK_MSG(IsValid(mHardwareDevice), "There is no suitable GPU device.");

        vk::PhysicalDeviceProperties deviceProperties = mHardwareDevice->GetVulkanPhysicalDevice().getProperties();
        std::string_view deviceName{deviceProperties.deviceName};
        TURBO_LOG(LOG_RHI, LOG_INFO, "Using \"{}\" as primary physical device. (Score: {})", deviceName, mHardwareDevice->CalculateDeviceScore());
    }
} // Turbo
