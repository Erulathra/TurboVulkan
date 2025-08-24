#pragma once

#include "CommandBuffer.h"
#include "VkBootstrap.h"

#include "ResourceBuilders.h"
#include "Core/DataStructures/DestoryQueue.h"
#include "Core/DataStructures/ResourcePool.h"
#include "Graphics/Resources.h"

namespace Turbo
{
	class FWindow;

	class FBufferedFrameData final
	{
		vk::Fence mCommandBufferExecutedFence = nullptr;
		vk::Semaphore mImageAcquiredSemaphore = nullptr;

		vk::CommandPool mCommandPool = nullptr;

		std::unique_ptr<FCommandBuffer> mCommandBuffer;
		FDescriptorPoolHandle mDescriptorPoolHandle;

	public:
		friend class FGPUDevice;
	};

	class FGPUDevice final
	{
		GENERATED_BODY(FGPUDevice)

		/** Initialization interface */
	public:
		void Init(const FGPUDeviceBuilder& gpuDeviceBuilder);
		void Shutdown();
		/** Initialization interface end */

		/** Rendering interface */
	public:
		void BeginFrame();
		void PresentFrame();

		FCommandBuffer* GetCommandBuffer() { return mFrameDatas[mBufferedFrameIndex].mCommandBuffer.get(); }

		// TODO: Remove me
		FTextureHandle GetPresentImage() { return mSwapChainTextures[mCurrentSwapchainImageIndex]; }

		/** Rendering interface end */

		/** Resource accessors */
	public:
		FBuffer* AccessBuffer(FBufferHandle handle) { return mBufferPool->Access(handle); }
		FTexture* AccessTexture(FTextureHandle handle) { return mTexturePool->Access(handle); }
		FSampler* AccessSampler(FSamplerHandle handle) { return mSamplerPool->Access(handle); }
		FPipeline* AccessPipeline(FPipelineHandle handle) { return mPipelinePool->Access(handle); }
		FDescriptorPool* AccessDescriptorPool(FDescriptorPoolHandle handle) { return mDescriptorPoolPool->Access(handle); }
		FDescriptorSetLayout* AccessDescriptorSetLayout(FDescriptorSetLayoutHandle handle) { return mDescriptorSetLayoutPool->Access(handle); }
		FDescriptorSet* AccessDescriptorSet(FDescriptorSetHandle handle) { return mDescriptorSetPool->Access(handle); }
		FShaderState* AccessShaderState(FShaderStateHandle handle) { return mShaderStatePool->Access(handle); }

		/** Resource accessors end */

		/** Initialization methods */
	private:
		vkb::Instance CreateVkInstance(const std::vector<cstring>& requiredExtensions);
		vkb::PhysicalDevice SelectPhysicalDevice(const vkb::Instance& builtInstance);
		vkb::Device CreateDevice(const vkb::PhysicalDevice& physicalDevice);
		vkb::Swapchain CreateSwapchain();
		void CreateVulkanMemoryAllocator();
		void CreateFrameDatas();
		vk::CommandPool CreateCommandPool();
		std::unique_ptr<FCommandBuffer> CreateCommandBuffer(vk::CommandPool commandPool);

		vk::PresentModeKHR GetBestPresentMode();
		/** Initialization methods end */

		/** Destroy methods */
	private:
		void DestroySwapChain();
		void DestroyFrameDatas();

		/** Destroy methods end */

		/** Rendering interface */
	private:
		void AdvanceFrameCounters();

		/** Rendering interface end */

	private:
		/** Debug */
		static VkBool32 ValidationLayerCallback(
			VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageType,
			const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
			void* userData
		);
		/** Debug end */

	private:
		/** Resource pools */
		TResourcePoolHeap<FBuffer, FBufferHandle, 4096> mBufferPool;
		TResourcePoolHeap<FTexture, FTextureHandle, 512> mTexturePool;
		TResourcePoolHeap<FSampler, FSamplerHandle, 32> mSamplerPool;
		TResourcePoolHeap<FPipeline, FPipelineHandle, 128> mPipelinePool;
		TResourcePoolHeap<FDescriptorSetLayout, FDescriptorSetLayoutHandle, 128> mDescriptorSetLayoutPool;
		TResourcePoolHeap<FDescriptorPool, FDescriptorPoolHandle, 16> mDescriptorPoolPool;
		TResourcePoolHeap<FDescriptorSet, FDescriptorSetHandle, 256> mDescriptorSetPool;
		TResourcePoolHeap<FShaderState, FShaderStateHandle, 128> mShaderStatePool;

		/** Resource pools end */

		/** Vulkan handles */
	private:
		vk::Instance mVkInstance = nullptr;

		vk::PhysicalDevice mVkPhysicalDevice = nullptr;
		vk::PhysicalDeviceProperties mPhysicalDeviceProperties = {};

		vk::Device mVkDevice = nullptr;
		vk::Queue mVkQueue = nullptr;
		uint32 mVkQueueFamilyIndex = std::numeric_limits<uint32>::max();

		vk::DescriptorPool mVkDescriptorPool = nullptr;

		vma::Allocator mVmaAllocator = nullptr;

		/** Vulkan Handles end */

		/** Swapchain */
	private:
		vk::SwapchainKHR mVkSwapchain = nullptr;
		vk::SurfaceKHR mVkWindowSurface = nullptr;
		vk::SurfaceFormatKHR mVkSurfaceFormat = {};
		vk::PresentModeKHR mPresentMode = {};

		std::array<FTextureHandle, kMaxSwapChainImages> mSwapChainTextures;
		std::array<vk::Semaphore, kMaxSwapChainImages> mSubmitSemaphores;

		uint32 mNumSwapChainImages = 0;
		/** Note that this is an index of swap chain image */
		uint32 mCurrentSwapchainImageIndex = 0;

		/** Swapchain end */

		/** Frame handing */
		std::array<FBufferedFrameData, kMaxBufferedFrames> mFrameDatas;

		/** Note that this is an index of buffered frame */
		uint32 mBufferedFrameIndex = 0;
		/** Note that this is an index of rendered frame (from Init) */
		uint32 mRenderedFrames = 0;

		/** TODO: move me to better category */
		bool mbVSync = false;

		/** Frame handing */

		/** Other */
	private:
		FDestroyQueue mDestroyQueue;
		vk::DebugUtilsMessengerEXT mVkDebugUtilsMessenger;


		/** Other end */

	private:
		FGPUDevice() = default;

	public:
		friend class FEngine;
	};
} // Turbo
