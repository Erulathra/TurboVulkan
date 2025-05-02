#pragma once
#include <vulkan/vulkan_core.h>

#define WITH_VALIDATION_LAYERS DEBUG

namespace Turbo
{
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
		static void Init();
		static void Destroy();

	private:
		void CreateVulkanInstance();
		void DestroyVulkanInstance();

	private:
		void EnumerateVulkanExtensions();

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

	private:
		static std::unique_ptr<VulkanRHI> Instance;

	private:
		VkInstance VulkanInstance = nullptr;
		std::vector<VkExtensionProperties> ExtensionProperties;

		VkPhysicalDevice PhysicalDevice = nullptr;

#if WITH_VALIDATION_LAYERS
		bool bValidationLayersEnabled = false;
		VkDebugUtilsMessengerEXT DebugMessengerHandle;
#endif // WITH_VALIDATION_LAYERS
	};
} // namespace Turbo
