#include "Core/VulkanRHI.h"

#include "Core/Window.h"

namespace Turbo {
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
		VkApplicationInfo AppInfo;
		AppInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		AppInfo.pApplicationName = "Turbo Vulkan";
		AppInfo.applicationVersion = TURBO_VERSION();
		AppInfo.pEngineName = "Turbo Vulkan";
		AppInfo.engineVersion = TURBO_VERSION();
		AppInfo.apiVersion = VK_API_VERSION_1_3;

		VkInstanceCreateInfo CreateInfo;
		CreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		CreateInfo.pApplicationInfo = &AppInfo;
		CreateInfo.pNext = nullptr;

		std::vector<const char*> ExtensionNames = Window::GetMain()->GetVulkanExtensions();

		CreateInfo.enabledExtensionCount = ExtensionNames.size();
		CreateInfo.ppEnabledExtensionNames = ExtensionNames.data();

		// TODO: Enable validation layers
		CreateInfo.enabledLayerCount = 0;
		CreateInfo.ppEnabledLayerNames = nullptr;

		TURBO_LOG(LOG_RHI, LOG_INFO, "Creating VKInstance.")
		const VkResult CreationResult = vkCreateInstance(&CreateInfo, nullptr, &VulkanContext.Instance);
		if (CreationResult != VK_SUCCESS)
		{
			TURBO_LOG(LOG_RHI, LOG_ERROR, "VKInstance creation failed. (Error: {})", static_cast<int32_t>(CreationResult));
			return;
		}
	}

	void VulkanRHI::DestroyVulkanInstance()
	{
		if (VulkanContext.Instance)
		{
			TURBO_LOG(LOG_RHI, LOG_INFO, "Destroying VKInstance...")
            vkDestroyInstance(VulkanContext.Instance, nullptr);
		}
	}
} // Turbo