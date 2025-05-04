#include "Core/VulkanRHI.h"

#include "Core/Engine.h"
#include "Core/Window.h"

namespace Turbo
{
    constexpr const char* vkCreateDebugUtilsMessengerEXT_FuncName = "vkCreateDebugUtilsMessengerEXT";
    constexpr const char* vkDestroyDebugUtilsMessengerEXT_FuncName = "vkDestroyDebugUtilsMessengerEXT";

    std::unique_ptr<VulkanRHI> VulkanRHI::Instance;

    VulkanRHI::VulkanRHI() = default;

    VulkanRHI::~VulkanRHI()
    {
    }

    void VulkanRHI::Init()
    {
        Instance = std::unique_ptr<VulkanRHI>(new VulkanRHI());

        Instance->CreateVulkanInstance();
        Instance->AcquirePhysicalDevice();
        Instance->CreateLogicalDevice();
    }

    void VulkanRHI::Destroy()
    {
        Instance->DestroyLogicalDevice();
        Instance->DestroyVulkanInstance();
        Instance.reset();
    }

    void VulkanRHI::CreateVulkanInstance()
    {
        VkApplicationInfo AppInfo{};
        AppInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        AppInfo.pApplicationName = "Turbo Vulkan";
        AppInfo.applicationVersion = TURBO_VERSION();
        AppInfo.pEngineName = "Turbo Vulkan";
        AppInfo.engineVersion = TURBO_VERSION();
        AppInfo.apiVersion = VK_API_VERSION_1_3;

        VkInstanceCreateInfo CreateInfo{};
        CreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        CreateInfo.pApplicationInfo = &AppInfo;
        CreateInfo.pNext = nullptr;

        std::vector<const char*> ExtensionNames = Window::GetMain()->GetVulkanExtensions();

#if WITH_VALIDATION_LAYERS
        if (CheckValidationLayersSupport())
        {
            TURBO_LOG(LOG_RHI, LOG_INFO, "Validation layers supported.")

            CreateInfo.enabledLayerCount = VulkanValidationLayers.size();
            CreateInfo.ppEnabledLayerNames = VulkanValidationLayers.data();

            ExtensionNames.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }
        else
#endif // else !WITH_VALIDATION_LAYERS
        {
            TURBO_LOG(LOG_RHI, LOG_INFO, "Validation layers are unsupported or disabled.")

            CreateInfo.enabledLayerCount = 0;
            CreateInfo.ppEnabledLayerNames = nullptr;
        }

        CreateInfo.enabledExtensionCount = ExtensionNames.size();
        CreateInfo.ppEnabledExtensionNames = ExtensionNames.data();

        TURBO_LOG(LOG_RHI, LOG_INFO, "Creating VKInstance.")
        const VkResult CreationResult = vkCreateInstance(&CreateInfo, nullptr, &VulkanInstance);
        if (CreationResult != VK_SUCCESS)
        {
            TURBO_LOG(LOG_RHI, LOG_ERROR, "VKInstance creation failed. (Error: {})", static_cast<int32>(CreationResult));
            Engine::Get()->RequestExit(EExitCode::RHICriticalError);
            return;
        }

#if WITH_VALIDATION_LAYERS
        SetupValidationLayersCallbacks();
#endif // WITH_VALIDATION_LAYERS

        EnumerateVulkanExtensions();

        std::stringstream ExtensionsStream;
        for (const VkExtensionProperties& Extension : ExtensionProperties)
        {
            ExtensionsStream << "\t" << Extension.extensionName << "\n";
        }
        TURBO_LOG(LOG_RHI, LOG_DISPLAY, "Supported Extensions: \n {}", ExtensionsStream.str());
    }

    void VulkanRHI::DestroyVulkanInstance()
    {
        if (VulkanInstance)
        {
#if WITH_VALIDATION_LAYERS
            DestroyValidationLayersCallbacks();
#endif // WITH_VALIDATION_LAYERS

            TURBO_LOG(LOG_RHI, LOG_INFO, "Destroying VKInstance...")
            vkDestroyInstance(VulkanInstance, nullptr);
            VulkanInstance = nullptr;
            PhysicalDevice = nullptr;
        }
    }

    void VulkanRHI::EnumerateVulkanExtensions()
    {
        TURBO_CHECK(VulkanInstance);

        uint32 ExtensionsNum;
        vkEnumerateInstanceExtensionProperties(nullptr, &ExtensionsNum, nullptr);
        ExtensionProperties.resize(ExtensionsNum);
        uint32 Result = vkEnumerateInstanceExtensionProperties(nullptr, &ExtensionsNum, ExtensionProperties.data());
        if (Result != VK_SUCCESS)
        {
            TURBO_LOG(LOG_RHI, LOG_ERROR, "Vulkan extensions enumeration error: {0:X}", Result);
        }
    }

#if WITH_VALIDATION_LAYERS

    bool VulkanRHI::CheckValidationLayersSupport()
    {
        uint32 LayerPropertiesNum;
        vkEnumerateInstanceLayerProperties(&LayerPropertiesNum, nullptr);

        std::vector<VkLayerProperties> LayerProperties;
        LayerProperties.reserve(LayerPropertiesNum);
        vkEnumerateInstanceLayerProperties(&LayerPropertiesNum, LayerProperties.data());

        bool bSuccess = true;

        for (const char* RequestedValidationLayer : VulkanValidationLayers)
        {
            for (VkLayerProperties LayerProperty : LayerProperties)
            {
                if (std::strcmp(LayerProperty.layerName, RequestedValidationLayer) != 0)
                {
                    TURBO_LOG(LOG_RHI, LOG_CRITICAL, "Missing Validation Layer: {}", RequestedValidationLayer);
                    bSuccess = false;
                }
            }
        }

        if (bSuccess)
        {
            TURBO_LOG(LOG_RHI, LOG_INFO, "All requested validation layers are supported.");
        }

        return bSuccess;
    }

    void VulkanRHI::SetupValidationLayersCallbacks()
    {
        TURBO_LOG(LOG_RHI, LOG_INFO, "Assigning validation layers callback.");

        VkDebugUtilsMessengerCreateInfoEXT CreateInfo{};
        CreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        CreateInfo.messageSeverity =
              VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

        CreateInfo.messageType =
              VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

        CreateInfo.pfnUserCallback = HandleValidationLayerCallback;
        CreateInfo.pUserData = nullptr;

        auto vkCreateDebugUtilsMessengerEXT_FuncPtr =
            reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr( VulkanInstance, vkCreateDebugUtilsMessengerEXT_FuncName));
        if (vkCreateDebugUtilsMessengerEXT_FuncPtr)
        {
            vkCreateDebugUtilsMessengerEXT_FuncPtr(VulkanInstance, &CreateInfo, nullptr, &DebugMessengerHandle);
        }
        else
        {
            TURBO_LOG(LOG_RHI, LOG_ERROR, "Cannot load {} function.", vkCreateDebugUtilsMessengerEXT_FuncName);
        }
    }

    void VulkanRHI::DestroyValidationLayersCallbacks()
    {
        if (DebugMessengerHandle)
        {
            TURBO_LOG(LOG_RHI, LOG_INFO, "Destroying validation layers callback.");

            auto vkDestroyDebugUtilsMessengerEXT_FuncPtr =
                reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(VulkanInstance, vkDestroyDebugUtilsMessengerEXT_FuncName));

            if (vkDestroyDebugUtilsMessengerEXT_FuncPtr)
            {
                vkDestroyDebugUtilsMessengerEXT_FuncPtr(VulkanInstance, DebugMessengerHandle, nullptr);
                DebugMessengerHandle = nullptr;
            }
            else
            {
                TURBO_LOG(LOG_RHI, LOG_ERROR, "Cannot load {} function.", vkCreateDebugUtilsMessengerEXT_FuncName);
            }
        }
    }


    VkBool32 VulkanRHI::HandleValidationLayerCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT MessageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT MessageType,
        const VkDebugUtilsMessengerCallbackDataEXT* CallbackData,
        void* UserData)
    {
        if (MessageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
        {
            TURBO_LOG(LOG_RHI, LOG_DISPLAY, "{}", CallbackData->pMessage)
        }
        else if (MessageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
        {
            // Messages with info severity are very verbose, so I reduced its verbosity to display.
            TURBO_LOG(LOG_RHI, LOG_DISPLAY, "{}", CallbackData->pMessage)
        }
        else if (MessageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        {
            TURBO_LOG(LOG_RHI, LOG_WARN, "{}", CallbackData->pMessage)
        }
        else if (MessageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
        {
            TURBO_LOG(LOG_RHI, LOG_ERROR, "{}", CallbackData->pMessage)
        }

        return VK_FALSE;
    }

#endif // WITH_VALIDATION_LAYERS

    void VulkanRHI::AcquirePhysicalDevice()
    {
        if (!VulkanInstance)
        {
            return;
        }

        uint32 PhysicalDeviceNum;
        vkEnumeratePhysicalDevices(VulkanInstance, &PhysicalDeviceNum, nullptr);

        if (PhysicalDeviceNum != 0)
        {
             std::vector<VkPhysicalDevice> FoundDevices(PhysicalDeviceNum);
            vkEnumeratePhysicalDevices(VulkanInstance, &PhysicalDeviceNum, FoundDevices.data());

            auto FilteredDevicesView = FoundDevices | std::views::filter([this](VkPhysicalDevice Device){ return IsDeviceCapable(Device); });
            std::vector FilteredFoundDevices(FilteredDevicesView.begin(), FilteredDevicesView.end());

            std::ranges::sort(
                FilteredFoundDevices, std::ranges::greater{},
                [this](VkPhysicalDevice Device)
                {
                    return CalculateDeviceScore(Device);
                });

            PhysicalDevice = !FilteredFoundDevices.empty() ? FilteredFoundDevices[0] : nullptr;
        }

        if (!PhysicalDevice)
        {
            TURBO_LOG(LOG_RHI, LOG_ERROR, "There is no suitable GPU device.");
            Engine::Get()->RequestExit(EExitCode::DeviceNotSupported);
            return;
        }

        VkPhysicalDeviceProperties DeviceProperties;
        vkGetPhysicalDeviceProperties(PhysicalDevice, &DeviceProperties);

        TURBO_LOG(LOG_RHI, LOG_INFO, "Using \"{}\" as primary physical device.", DeviceProperties.deviceName);
    }

    int32 VulkanRHI::CalculateDeviceScore(VkPhysicalDevice Device)
    {
        VkPhysicalDeviceProperties DeviceProperties;
        VkPhysicalDeviceFeatures DeviceFeatures;
        VkPhysicalDeviceMemoryProperties MemoryProperties;

        vkGetPhysicalDeviceProperties(Device, &DeviceProperties);
        vkGetPhysicalDeviceFeatures(Device, &DeviceFeatures);
        vkGetPhysicalDeviceMemoryProperties(Device, &MemoryProperties);

        int32 DeviceScore = 0;
        DeviceScore += -1024 * (DeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU);
        DeviceScore += -1024 * (DeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_CPU);
        DeviceScore += 1024 * (DeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU);

        return DeviceScore;
    }

    bool VulkanRHI::IsDeviceCapable(VkPhysicalDevice Device)
    {
        bool bResult = true;
        bResult &= FindQueueFamilies(Device).IsValid();

        return bResult;
    }

    bool VulkanRHI::QueueFamilyIndices::IsValid() const
    {
        return GraphicsFamily.has_value();
    }

    VulkanRHI::QueueFamilyIndices VulkanRHI::FindQueueFamilies(VkPhysicalDevice Device)
    {
        QueueFamilyIndices Result;

        if (TURBO_ENSURE(Device))
        {
            uint32 QueueFamilyNum;
            vkGetPhysicalDeviceQueueFamilyProperties(Device, &QueueFamilyNum, nullptr);

            std::vector<VkQueueFamilyProperties> QueueFamilyProperties(QueueFamilyNum);
            vkGetPhysicalDeviceQueueFamilyProperties(Device, &QueueFamilyNum, QueueFamilyProperties.data());

            for (int QueueId = 0; QueueId < QueueFamilyNum; ++QueueId)
            {
                const VkQueueFamilyProperties& FamilyProperties = QueueFamilyProperties[QueueId];
                if (FamilyProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT)
                {
                    Result.GraphicsFamily = QueueId;
                }
            }
        }

        return Result;
    }

    void VulkanRHI::CreateLogicalDevice()
    {
        if (!VulkanInstance || !PhysicalDevice)
        {
            return;
        }

        TURBO_LOG(LOG_RHI, LOG_INFO, "Creating logical device.")

        QueueFamilyIndices QueueIndices = FindQueueFamilies(PhysicalDevice);
        if (!QueueIndices.IsValid())
        {
            TURBO_LOG(LOG_RHI, LOG_ERROR, "Cannot obtain queue family indices.")
            return;
        }

        VkDeviceQueueCreateInfo QueueCreateInfo{};
        QueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        QueueCreateInfo.queueFamilyIndex = *QueueIndices.GraphicsFamily;
        QueueCreateInfo.queueCount = 1;

        constexpr float QueuePriority = 1.f;
        QueueCreateInfo.pQueuePriorities = &QueuePriority;

        VkDeviceCreateInfo DeviceCreateInfo{};
        DeviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        DeviceCreateInfo.pQueueCreateInfos = &QueueCreateInfo;
        DeviceCreateInfo.queueCreateInfoCount = 1;

        VkPhysicalDeviceFeatures DeviceFeatures = GetRequiredDeviceFeatures();
        DeviceCreateInfo.pEnabledFeatures = &DeviceFeatures;

        const VkResult DeviceCreationResult = vkCreateDevice(PhysicalDevice, &DeviceCreateInfo, nullptr, &Device);
        if (DeviceCreationResult != VK_SUCCESS)
        {
            TURBO_LOG(LOG_RHI, LOG_ERROR, "Error: {} during creating logical device.", static_cast<int32>(DeviceCreationResult));
            Engine::Get()->RequestExit(EExitCode::RHICriticalError);
            return;
        }
    }

    void VulkanRHI::DestroyLogicalDevice()
    {
        vkDestroyDevice(Device, nullptr);
        Device = nullptr;
    }

    VkPhysicalDeviceFeatures VulkanRHI::GetRequiredDeviceFeatures()
    {
        VkPhysicalDeviceFeatures Result{};
        Result.textureCompressionBC = true;

        return Result;
    }

    bool VulkanRHI::AcquiredQueues::IsValid() const
    {
        return GraphicsQueue;
    }

    void VulkanRHI::SetupQueues()
    {
        if (!PhysicalDevice || !Device)
        {
            return;
        }

        QueueFamilyIndices QueueIndices = FindQueueFamilies(PhysicalDevice);
        if (QueueIndices.IsValid())
        {
            vkGetDeviceQueue(Device, *QueueIndices.GraphicsFamily, 0, &Queues.GraphicsQueue);
        }

        if (!Queues.IsValid())
        {
            TURBO_LOG(LOG_RHI, LOG_ERROR, "Required queues not found.")
            Engine::Get()->RequestExit(EExitCode::RHICriticalError);
            return;
        }
    }
} // Turbo
