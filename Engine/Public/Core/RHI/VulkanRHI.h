#pragma once

#include "RHIDestoryQueue.h"
#include "RHICore.h"
#include "FrameData.h"
#include "SwapChain.h"

#define WITH_VALIDATION_LAYERS DEBUG

#define VULKAN_VERSION VK_API_VERSION_1_3

namespace Turbo
{
	class FComputePipeline;
	class FDescriptorAllocator;
	class FImage;
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
		void InitDrawImage();
		void InitDescriptors();

		// TODO: remove me
		void InitScene();

		void RenderSync();
		void Tick();

	private:
		void AcquireSwapChainImage();
		void DrawFrame();
		void BlitDrawImageToSwapchainImage(const vk::CommandBuffer& cmd);
		void DrawScene(const vk::CommandBuffer& cmd);
		void PresentImage();

	public:
		[[nodiscard]] uint64 GetFrameNumber() const { return mFrameNumber; }
		[[nodiscard]] uint32 GetFrameDataIndex() const;
		[[nodiscard]] FFrameData& GetCurrentFrame() { return mFrameDatas[GetFrameDataIndex()]; }

		[[nodiscard]] FRHIDestroyQueue& GetFrameDeletionQueue() { return GetCurrentFrame().GetDeletionQueue(); }

		[[nodiscard]] FImage& GetDrawImage() const { TURBO_CHECK(mDrawImage); return *mDrawImage; }

	private:
		std::vector<FFrameData> mFrameDatas;
		std::unique_ptr<FImage> mDrawImage;

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

		/** ImGui */
	private:
		void InitImGui();
		void BeginImGuiFrame();
		void DrawImGuiFrame(const vk::CommandBuffer& cmd, const vk::ImageView& targetImageView);
		void DestroyImGui();

	private:
		std::unique_ptr<FDescriptorAllocator> mImGuiAllocator;

		/** ImGui end */

	public:
		[[nodiscard]] vk::Instance GetVulkanInstance() const { return mVulkanInstance; }
		[[nodiscard]] FRHIDestroyQueue& GetMainDeletionQueue() { return mMainDestroyQueue; }

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

		std::unique_ptr<FDescriptorAllocator> mMainDescriptorAllocator;
		FRHIDestroyQueue mMainDestroyQueue;

		/** TODO: REMOVE ME */
		std::unique_ptr<FComputePipeline> mComputePipeline;
		/** TODO: REMOVE ME END */

	public:
		[[nodiscard]] FVulkanHardwareDevice* GetHardwareDevice() const { return mHardwareDevice.get(); }
		[[nodiscard]] FVulkanDevice* GetDevice() const { return mDevice.get(); }
		[[nodiscard]] FSwapChain* GetSwapChainInstance() const { return mSwapChain.get(); }

	public:
		friend class FEngine;
	};
} // namespace Turbo
