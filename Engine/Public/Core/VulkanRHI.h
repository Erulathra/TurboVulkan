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
			VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageType,
			const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
			void* pUserData);
#endif // WITH_VALIDATION_LAYERS

	private:
		static std::unique_ptr<VulkanRHI> Instance;

	private:
		struct
		{
			VkInstance Instance = VK_NULL_HANDLE;

			std::vector<VkExtensionProperties> ExtensionProperties;

#if WITH_VALIDATION_LAYERS
			bool bValidationLayersEnabled = false;

			VkDebugUtilsMessengerEXT DebugMessengerHandle;
#endif // WITH_VALIDATION_LAYERS
		} VulkanContext;
	};
} // namespace Turbo
