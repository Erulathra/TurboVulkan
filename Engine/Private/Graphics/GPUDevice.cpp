#include "Graphics/GPUDevice.h"

#include "VkBootstrap.h"

#include "Core/Window.h"

namespace Turbo
{
	void FGPUDevice::Init(const FGPUDeviceBuilder& gpuDeviceBuilder)
	{
		TURBO_LOG(LOG_GPU_DEVICE, Info, "Initializing GPU Device.");

		gpuDeviceBuilder.mWindow->InitForVulkan();

		VULKAN_HPP_DEFAULT_DISPATCHER.init();

		vkb::InstanceBuilder instanceBuilder;
		instanceBuilder
			.set_app_name("TurboEngine")
			.set_app_version(TURBO_VERSION())
#if WITH_VALIDATION_LAYERS
			.request_validation_layers(true)
			.set_debug_callback(&ThisClass::ValidationLayerCallback)
			.set_debug_messenger_severity(VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
#endif // WITH_VALIDATION_LAYERS
			.set_minimum_instance_version(1, 3, 0);

		vkb::Result<vkb::Instance> BuildInstanceResult = instanceBuilder.build();
		TURBO_CHECK_MSG(BuildInstanceResult, "Vulkan Instance Creation failed. Reason: {}", BuildInstanceResult.error().message())

		vkb::Instance vkbInstance = BuildInstanceResult.value();
		mVkInstance = vkbInstance;
		VULKAN_HPP_DEFAULT_DISPATCHER.init(mVkInstance);

		vkb::PhysicalDeviceSelector physicalDeviceSelector(vkbInstance);

	}

	void FGPUDevice::Shutdown()
	{
	}

	VkBool32 FGPUDevice::ValidationLayerCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
		void* userData
		)
	{
		if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
		{
			TURBO_LOG(LOG_RHI, Display, "{}", callbackData->pMessage)
		}
		else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
		{
			// Messages with info severity are very verbose, so I reduced its verbosity to display.
			TURBO_LOG(LOG_RHI, Display, "{}", callbackData->pMessage)
		}
		else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
		{
			TURBO_LOG(LOG_RHI, Warn, "{}", callbackData->pMessage)
		}
		else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
		{
			TURBO_LOG(LOG_RHI, Error, "{}", callbackData->pMessage)
			TURBO_DEBUG_BREAK();
		}

		return VK_FALSE;
	}
} // Turbo