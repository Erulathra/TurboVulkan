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
	public:
		struct QueueFamilyIndices
		{
			std::optional<int32> GraphicsFamily;

			[[nodiscard]] bool IsValid() const;
		};


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

#pragma region Validation Layers
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
#pragma endregion

#pragma region Physical Device
	private:
		void AcquirePhysicalDevice();

		int32 CalculateDeviceScore(VkPhysicalDevice Device);
		bool IsDeviceCapable(VkPhysicalDevice Device);
		QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice Device);

#pragma endregion

#pragma region Logical Device
	private:
		void CreateLogicalDevice();
		void DestroyLogicalDevice();
		VkPhysicalDeviceFeatures GetRequiredDeviceFeatures();

#pragma endregion

	private:
		struct AcquiredQueues
		{
			VkQueue GraphicsQueue;

			[[nodiscard]] bool IsValid() const;
		} Queues;

		void SetupQueues();

	private:
		static std::unique_ptr<VulkanRHI> Instance;

	private:
		VkInstance VulkanInstance = nullptr;
		std::vector<VkExtensionProperties> ExtensionProperties;

		VkPhysicalDevice PhysicalDevice = nullptr;
		VkDevice Device = nullptr;

#if WITH_VALIDATION_LAYERS
		bool bValidationLayersEnabled = false;
		VkDebugUtilsMessengerEXT DebugMessengerHandle;
#endif // WITH_VALIDATION_LAYERS
	};
} // namespace Turbo
