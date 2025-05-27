#pragma once

#include "RHICore.h"

#define WITH_VALIDATION_LAYERS DEBUG

#define CHECK_VULKAN_RESULT(RESULT, MESSAGE)									\
if (RESULT != VK_SUCCESS)														\
{																				\
	TURBO_LOG(LOG_RHI, LOG_ERROR, MESSAGE, RESULT);								\
	gEngine->RequestExit(EExitCode::RHICriticalError);							\
	return;																		\
}

template <> struct fmt::formatter<VkResult>: formatter<int32> {
	auto format(VkResult Result, format_context& CTX) const
	  -> format_context::iterator;
};

namespace Turbo
{
	class SwapChain;
	class Window;
	class Device;
	class HardwareDevice;

#if WITH_VALIDATION_LAYERS
	// TODO: Move to config
	const static std::vector<const char*> VulkanValidationLayers =
	{
		"VK_LAYER_KHRONOS_validation"
	};
#endif // WITH_VALIDATION_LAYERS

	class VulkanRHI
	{
	private:
		explicit VulkanRHI();

	public:
		~VulkanRHI();

	public:
		void Init();
		void InitWindow(Window* Window);
		void Destroy();

	private:
		void CreateVulkanInstance();
		void DestroyVulkanInstance();

	private:
		void EnumerateVulkanExtensions();

/** Validation Layers */
#if WITH_VALIDATION_LAYERS

	private:
		bool CheckValidationLayersSupport();

		void SetupValidationLayersCallbacks();
		void DestroyValidationLayersCallbacks();

		static VKAPI_ATTR VkBool32 VKAPI_CALL HandleValidationLayerCallback(
			VkDebugUtilsMessageSeverityFlagBitsEXT MessageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT MessageType,
			const VkDebugUtilsMessengerCallbackDataEXT* CallbackData,
			void* UserData);
#endif // WITH_VALIDATION_LAYERS
/** Validation Layers End */

	public:
		[[nodiscard]] VkInstance GetVulkanInstance() const { return VulkanInstance; }

	private:
		void AcquirePhysicalDevice();

	private:
		VkInstance VulkanInstance = nullptr;
		std::vector<VkExtensionProperties> ExtensionProperties;

#if WITH_VALIDATION_LAYERS
		bool bValidationLayersEnabled = false;
		VkDebugUtilsMessengerEXT DebugMessengerHandle;
#endif // WITH_VALIDATION_LAYERS

	private:
		std::unique_ptr<HardwareDevice> HardwareDeviceInstance;
		std::unique_ptr<Device> DeviceInstance;
		std::unique_ptr<SwapChain> SwapChainInstance;

	public:
		[[nodiscard]] HardwareDevice* GetHardwareDevice() const { return HardwareDeviceInstance.get(); }
		[[nodiscard]] Device* GetDevice() const { return DeviceInstance.get(); }
		[[nodiscard]] SwapChain* GetSwapChainInstance() const { return SwapChainInstance.get(); }

	public:
		friend class Engine;
	};
} // namespace Turbo
