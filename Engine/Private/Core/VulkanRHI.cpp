#include "Core/VulkanRHI.h"

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
    }

    void VulkanRHI::Destroy()
    {
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
            TURBO_LOG(LOG_RHI, LOG_INFO, "All requested validation layer supported.");
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
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData)
    {
        auto LogVerbosity = LOG_ERROR;
        if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
        {
            LogVerbosity = LOG_DISPLAY;
        }
        else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
        {
            LogVerbosity = LOG_INFO;
        }
        else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        {
            LogVerbosity = LOG_WARN;
        }
        else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
        {
            LogVerbosity = LOG_ERROR;
        }

        TURBO_LOG(LOG_RHI, LOG_ERROR, "Vulkan Error: {}", pCallbackData->pMessage)

        return VK_FALSE;
    }

#endif // WITH_VALIDATION_LAYERS
} // Turbo
