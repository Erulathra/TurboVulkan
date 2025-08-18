#include "Graphics/GPUDevice.h"

#include "VkBootstrap.h"
#include "Core/Engine.h"

#include "Core/Window.h"

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

namespace Turbo
{
	void FGPUDevice::Init(const FGPUDeviceBuilder& gpuDeviceBuilder)
	{
		TURBO_LOG(LOG_GPU_DEVICE, Info, "Initializing GPU Device.");

		gpuDeviceBuilder.mWindow->InitForVulkan();
		gpuDeviceBuilder.mWindow->Init();

		VULKAN_HPP_DEFAULT_DISPATCHER.init();

		vkb::InstanceBuilder instanceBuilder;
		instanceBuilder
			.set_app_name("TurboEngine")
			.set_app_version(TURBO_VERSION())
#if WITH_VALIDATION_LAYERS
			.request_validation_layers(true)
			.set_debug_callback(&ThisClass::ValidationLayerCallback)
			// .set_debug_messenger_severity(VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
#endif // WITH_VALIDATION_LAYERS
			.set_minimum_instance_version(1, 3, 0);

		vkb::Result<vkb::Instance> buildInstanceResult = instanceBuilder.build();
		TURBO_CHECK_MSG(buildInstanceResult, "Vulkan Instance Creation failed. Reason: {}", buildInstanceResult.error().message())

		mVkInstance = buildInstanceResult.value();
		mVkDebugUtilsMessenger = buildInstanceResult.value().debug_messenger;
		VULKAN_HPP_DEFAULT_DISPATCHER.init(mVkInstance);

		TURBO_CHECK(gpuDeviceBuilder.mWindow->CreateVulkanSurface(mVkInstance));
		VkSurfaceKHR vkSurface = gpuDeviceBuilder.mWindow->GetVulkanSurface();

		vk::PhysicalDeviceFeatures deviceFeatures;
		deviceFeatures.setTextureCompressionBC(true);

		vk::PhysicalDeviceVulkan12Features device12Features;
		device12Features.setBufferDeviceAddress(true);
		device12Features.setDescriptorIndexing(true);

		vk::PhysicalDeviceVulkan13Features device13Features;
		device13Features.setDynamicRendering(true);
		device13Features.setSynchronization2(true);

		vkb::PhysicalDeviceSelector physicalDeviceSelector(buildInstanceResult.value());
		physicalDeviceSelector
			.set_surface(vkSurface)
			.require_present(true)
			.set_required_features(deviceFeatures)
			.set_required_features_12(device12Features)
			.set_required_features_13(device13Features)
			.set_minimum_version(1, 3)
			.prefer_gpu_device_type(vkb::PreferredDeviceType::discrete)
			.add_required_extension(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

		vkb::Result<vkb::PhysicalDevice> selectPhysicalDeviceResult = physicalDeviceSelector.select();
		TURBO_CHECK_MSG(selectPhysicalDeviceResult, "Physical device selection failed. Reason: {}", selectPhysicalDeviceResult.error().message())

		vkb::PhysicalDevice selectedPhysicalDevice = selectPhysicalDeviceResult.value();

		mVkPhysicalDevice = selectedPhysicalDevice;
		TURBO_LOG(LOG_GPU_DEVICE, Info, "Selected {} as primary physical device.", selectedPhysicalDevice.name);

		vkb::DeviceBuilder deviceBuilder(selectedPhysicalDevice);
		vkb::Result<vkb::Device> buildDeviceResult = deviceBuilder.build();
		TURBO_CHECK_MSG(buildDeviceResult, "Device creation failed. Reason: {}", buildInstanceResult.error().message())

		mVkDevice = buildDeviceResult.value();
	}

	void FGPUDevice::Shutdown()
	{
		CHECK_VULKAN_HPP(mVkDevice.waitIdle())

		TURBO_LOG(LOG_GPU_DEVICE, Info, "Starting Gpu Device shutdown.")

		if (mVkDevice)
		{
			mVkDevice.destroy();
		}

		gEngine->GetWindow()->DestroyVulkanSurface(mVkInstance);

		if (mVkInstance)
		{
			mVkInstance.destroyDebugUtilsMessengerEXT(mVkDebugUtilsMessenger);
			mVkInstance.destroy();
		}
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