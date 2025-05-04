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
			int32 GraphicsFamily = INDEX_NONE;
			int32 PresentFamily = INDEX_NONE;

			[[nodiscard]] bool IsValid() const;
			[[nodiscard]] std::set<uint32> GetUniqueQueueIndices() const;
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

/** Surface */
	private:
		void CreateSurface();
		void DestroySurface();

/** Surface end */

/** Physical Device */
	private:
		void AcquirePhysicalDevice();

		int32 CalculateDeviceScore(VkPhysicalDevice Device);
		bool IsDeviceCapable(VkPhysicalDevice Device);
		QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice Device) const;

/** Physical Device end */

/** Logical Device */
	private:
		void CreateLogicalDevice();
		void DestroyLogicalDevice();
		VkPhysicalDeviceFeatures GetRequiredDeviceFeatures();

/** Logical Device End */

	private:
		struct AcquiredQueues
		{
			VkQueue GraphicsQueue;
			VkQueue PresentQueue;

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

		VkSurfaceKHR Surface = nullptr;

#if WITH_VALIDATION_LAYERS
		bool bValidationLayersEnabled = false;
		VkDebugUtilsMessengerEXT DebugMessengerHandle;
#endif // WITH_VALIDATION_LAYERS
	};
} // namespace Turbo
