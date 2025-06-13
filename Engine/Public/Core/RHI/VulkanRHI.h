#pragma once

#include "RHICore.h"
#include "FrameData.h"
#include "SwapChain.h"

#define WITH_VALIDATION_LAYERS DEBUG

namespace Turbo
{
	class FSwapChain;
	class FSDLWindow;
	class FVulkanDevice;
	class FVulkanHardwareDevice;

#if WITH_VALIDATION_LAYERS
	// TODO: Move to config
	const static std::vector<const char*> kVulkanValidationLayers =
	{
		"VK_LAYER_KHRONOS_validation"
	};
#endif // WITH_VALIDATION_LAYERS

	class FVulkanRHI
	{
	private:
		explicit FVulkanRHI();

	public:
		~FVulkanRHI();

	public:
		void Init();
		void InitWindow(FSDLWindow* window);
		void Destroy();

	private:
		void CreateVulkanInstance();
		void DestroyVulkanInstance();

		/** Frame rendering */
	public:
		void InitFrameData();

		void RenderSync();
		void Tick();

	private:
		void AcquireSwapChainImage();
		void DrawFrame();
		void PresentImage();

	public:
		uint64 GetFrameNumber() const { return mFrameNumber; }
		uint32 GetFrameDataIndex() const;
		const FFrameData& GetCurrentFrame() const { return mFrameDatas[GetFrameDataIndex()]; }

	private:
		std::vector<FFrameData> mFrameDatas;

		uint32 mSwapChainImageIndex = 0;
		uint64 mFrameNumber = 0;

		/** Frame rendering end */

/** Validation Layers */
#if WITH_VALIDATION_LAYERS

	private:
		bool CheckValidationLayersSupport();

		void SetupValidationLayersCallbacks();
		void DestroyValidationLayersCallbacks();

		static VKAPI_ATTR vk::Bool32 VKAPI_CALL HandleValidationLayerCallback(
			vk::DebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			vk::DebugUtilsMessageTypeFlagsEXT messageType,
			const vk::DebugUtilsMessengerCallbackDataEXT* callbackData,
			void* userData);
#endif // WITH_VALIDATION_LAYERS
/** Validation Layers End */

	public:
		[[nodiscard]] vk::Instance GetVulkanInstance() const { return mVulkanInstance; }

	private:
		void AcquirePhysicalDevice();

	private:
		vk::Instance mVulkanInstance = nullptr;
		std::vector<vk::ExtensionProperties> mExtensionProperties;

#if WITH_VALIDATION_LAYERS
		bool mbValidationLayersEnabled = false;
		vk::DebugUtilsMessengerEXT mDebugMessengerHandle;
#endif // WITH_VALIDATION_LAYERS

	private:
		std::unique_ptr<FVulkanHardwareDevice> mHardwareDevice;
		std::unique_ptr<FVulkanDevice> mDevice;
		std::unique_ptr<FSwapChain> mSwapChain;

	public:
		[[nodiscard]] FVulkanHardwareDevice* GetHardwareDevice() const { return mHardwareDevice.get(); }
		[[nodiscard]] FVulkanDevice* GetDevice() const { return mDevice.get(); }
		[[nodiscard]] FSwapChain* GetSwapChainInstance() const { return mSwapChain.get(); }

	public:
		friend class FEngine;
	};
} // namespace Turbo
