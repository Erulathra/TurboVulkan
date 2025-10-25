#pragma once

#include "CommandBuffer.h"
#include "VkBootstrap.h"

#include "ResourceBuilders.h"
#include "DestoryQueue.h"
#include "Graphics/Resources.h"
#include "VulkanHelpers.h"

namespace Turbo
{
	class FWindow;

	DECLARE_DELEGATE(FOnImmediateSubmit, FCommandBuffer&);

	class FBufferedFrameData final
	{
		vk::Fence mCommandBufferExecutedFence = nullptr;
		vk::Semaphore mImageAcquiredSemaphore = nullptr;

		vk::CommandPool mCommandPool = nullptr;

		TUniquePtr<FCommandBuffer> mCommandBuffer;
		THandle<FDescriptorPool> mDescriptorPoolHandle;

		FDestroyQueue mDestroyQueue;

	public:
		friend class FGPUDevice;
	};

	class FGPUDevice final
	{
		/** Initialization interface */
	public:
		void Init(const FGPUDeviceBuilder& gpuDeviceBuilder);
		void Shutdown();
		/** Initialization interface end */

		/** Rendering interface */
	public:
		bool BeginFrame();
		bool PresentFrame();

		FCommandBuffer* GetCommandBuffer() { return mFrameDatas[mBufferedFrameIndex].mCommandBuffer.get(); }
		THandle<FDescriptorPool> GetDescriptorPool() { return mFrameDatas[mBufferedFrameIndex].mDescriptorPoolHandle; }

		THandle<FTexture> GetPresentImage() { return mSwapChainTextures[mCurrentSwapchainImageIndex]; }

		void WaitIdle() const;

		void ImmediateSubmit(const FOnImmediateSubmit& immediateSubmitDelegate);

		/** Rendering interface end */

		/** Resource accessors */
	public:
		[[nodiscard]] FBuffer* AccessBuffer(THandle<FBuffer> handle) { return mBufferPool->Access(handle); }
		[[nodiscard]] FTexture* AccessTexture(THandle<FTexture> handle) { return mTexturePool->Access(handle); }
		[[nodiscard]] FSampler* AccessSampler(THandle<FSampler> handle) { return mSamplerPool->Access(handle); }
		[[nodiscard]] FPipeline* AccessPipeline(THandle<FPipeline> handle) { return mPipelinePool->Access(handle); }
		[[nodiscard]] FDescriptorPool* AccessDescriptorPool(THandle<FDescriptorPool> handle) { return mDescriptorPoolPool->Access(handle); }
		[[nodiscard]] FDescriptorSetLayout* AccessDescriptorSetLayout(THandle<FDescriptorSetLayout> handle) { return mDescriptorSetLayoutPool->Access(handle); }
		[[nodiscard]] FDescriptorSet* AccessDescriptorSet(THandle<FDescriptorSet> handle) { return mDescriptorSetPool->Access(handle); }
		[[nodiscard]] FShaderState* AccessShaderState(THandle<FShaderState> handle) { return mShaderStatePool->Access(handle); }

		[[nodiscard]] const FBuffer* AccessBuffer(THandle<FBuffer> handle) const { return mBufferPool->Access(handle); }
		[[nodiscard]] const FTexture* AccessTexture(THandle<FTexture> handle) const { return mTexturePool->Access(handle); }
		[[nodiscard]] const FSampler* AccessSampler(THandle<FSampler> handle) const { return mSamplerPool->Access(handle); }
		[[nodiscard]] const FPipeline* AccessPipeline(THandle<FPipeline> handle) const { return mPipelinePool->Access(handle); }
		[[nodiscard]] const FDescriptorPool* AccessDescriptorPool(THandle<FDescriptorPool> handle) const { return mDescriptorPoolPool->Access(handle); }
		[[nodiscard]] const FDescriptorSetLayout* AccessDescriptorSetLayout(THandle<FDescriptorSetLayout> handle) const { return mDescriptorSetLayoutPool->Access(handle); }
		[[nodiscard]] const FDescriptorSet* AccessDescriptorSet(THandle<FDescriptorSet> handle) const { return mDescriptorSetPool->Access(handle); }
		[[nodiscard]] const FShaderState* AccessShaderState(THandle<FShaderState> handle) const { return mShaderStatePool->Access(handle); }

		/** Resource accessors end */

		/** Resource creation */
	public:
		THandle<FBuffer> CreateBuffer(const FBufferBuilder& builder, FCommandBuffer* commandBuffer = nullptr);
		THandle<FTexture> CreateTexture(const FTextureBuilder& builder);
		THandle<FPipeline> CreatePipeline(const FPipelineBuilder& builder);
		THandle<FDescriptorPool> CreateDescriptorPool(const FDescriptorPoolBuilder& builder);
		THandle<FDescriptorSetLayout> CreateDescriptorSetLayout(const FDescriptorSetLayoutBuilder& builder);
		THandle<FDescriptorSet> CreateDescriptorSet(const FDescriptorSetBuilder& builder);
		THandle<FShaderState> CreateShaderState(const FShaderStateBuilder& builder);

		/** Resource creation end */

		/** Other resource related methods */
	public:
		void ResetDescriptorPool(THandle<FDescriptorPool> descriptorPoolHandle);
		/** Other resource related methods end */

		/** Resource destroy */
	public:
		void DestroyBuffer(THandle<FBuffer> handle);
		void DestroyTexture(THandle<FTexture> handle);
		void DestroyPipeline(THandle<FPipeline> handle);
		void DestroyDescriptorPool(THandle<FDescriptorPool> handle);
		void DestroyDescriptorSetLayout(THandle<FDescriptorSetLayout> handle);
		void DestroyShaderState(THandle<FShaderState> handle);

		/** Resource destroy end */

		/** Destroy immediate */
	public:
		void DestroyBufferImmediate(const FBufferDestroyer& destroyer);
		void DestroyTextureImmediate(const FTextureDestroyer& destroyer);
		void DestroyPipelineImmediate(const FPipelineDestroyer& destroyer);
		void DestroyDescriptorPoolImmediate(const FDescriptorPoolDestroyer& destroyer);
		void DestroyDescriptorSetLayoutImmediate(const FDescriptorSetLayoutDestroyer& destroyer);
		void DestroyShaderStateImmediate(const FShaderStateDestroyer& destroyer);

		/** Destroy immediate end */


		/** Vulkan Getters */
	public:
		[[nodiscard]] vk::Instance GetVkInstance() const { return mVkInstance; }
		[[nodiscard]] vk::PhysicalDevice GetVkPhysicalDevice() const { return mVkPhysicalDevice; }
		[[nodiscard]] vk::Device GetVkDevice() const { return mVkDevice; }
		[[nodiscard]] vk::Queue GetVkQueue() const { return mVkQueue; }

		/** Vulkan Getters end */

#if WITH_PROFILER
		/** Profiling */
		[[nodiscard]] FTraceGPUCtx GetTraceGpuCtx() const { return mTraceGpuCtx; }
#endif

		/** Initialization methods */
	private:
		vkb::Instance CreateVkInstance(const std::vector<cstring>& requiredExtensions);
		vkb::PhysicalDevice SelectPhysicalDevice(const vkb::Instance& builtInstance);
		vkb::Device CreateDevice(const vkb::PhysicalDevice& physicalDevice);
		vkb::Swapchain CreateSwapchain();
		void CreateVulkanMemoryAllocator();
		void CreateFrameDatas();
		vk::CommandPool CreateCommandPool(vk::CommandPoolCreateFlags createFlags = {});
		TUniquePtr<FCommandBuffer> CreateCommandBuffer(vk::CommandPool commandPool, FName name = FName());

		void InitializeImmediateCommands();

		vk::PresentModeKHR GetBestPresentMode();
		/** Initialization methods end */

	private:
		void ResizeSwapChain();

		/** Destroy methods */
	private:
		void DestroySwapChain();
		void DestroyFrameDatas();
		void DestroyImmediateCommands();

		/** Destroy methods end */

		/** Utils */
	private:
		// void CreateStagingBuffer
		/** Utils end */

		/** Rendering interface */
	private:
		void AdvanceFrameCounters();

		/** Rendering interface end */

		/** Creation helpers */
	private:
		void InitVulkanTexture(const FTextureBuilder& builder, THandle<FTexture> handle, FTexture* texture);

		/** Creation helpers end */

		/** Debug */
	private:
		static VkBool32 ValidationLayerCallback(
			VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageType,
			const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
			void* userData
		);

		template<typename HandleType>
		void SetResourceName(HandleType vkHandle, FName name) const;

		template<typename HandleType>
		void SetResourceName(HandleType vkHandle, std::string_view name) const;
		/** Debug end */

	private:
		/** Resource pools */
		TPoolHeap<FBuffer, 4096> mBufferPool;
		TPoolHeap<FTexture, 512> mTexturePool;
		TPoolHeap<FSampler, 32> mSamplerPool;
		TPoolHeap<FPipeline, 128> mPipelinePool;
		TPoolHeap<FDescriptorSetLayout, 128> mDescriptorSetLayoutPool;
		TPoolHeap<FDescriptorPool, 16> mDescriptorPoolPool;
		TPoolHeap<FDescriptorSet, 256> mDescriptorSetPool;
		TPoolHeap<FShaderState, 128> mShaderStatePool;

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

		std::array<THandle<FTexture>, kMaxSwapChainImages> mSwapChainTextures;
		std::array<vk::Semaphore, kMaxSwapChainImages> mSubmitSemaphores;

		uint32 mNumSwapChainImages = 0;
		/** Note that this is an index of swap chain image */
		uint32 mCurrentSwapchainImageIndex = 0;

		bool mbRequestedSwapchainResize = false;

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

		/** Immediate commands */
	private:
		vk::Fence mImmediateCommandsFence;
		vk::CommandPool mImmediateCommandsPool;
		TUniquePtr<FCommandBuffer> mImmediateCommandsBuffer;

		/** Immediate commands end */

		/** Profiling */
	private:
		FTraceGPUCtx mTraceGpuCtx;
		/** Profiling end */

		/** Other */
	private:
		FDestroyQueue mDestroyQueue;
		vk::DebugUtilsMessengerEXT mVkDebugUtilsMessenger;

		/** Other end */

	private:
		FGPUDevice() = default;

	public:
		DELETE_COPY(FGPUDevice);

	public:
		friend class FEngine;
	};


	template <typename HandleType>
	void FGPUDevice::SetResourceName(HandleType vkHandle, FName name) const
	{
		SetResourceName(vkHandle, name.ToString());
	}

	template <typename HandleType>
	void FGPUDevice::SetResourceName(HandleType vkHandle, const std::string_view name) const
	{
#if WITH_DEBUG_RENDERING_FEATURES
		vk::DebugUtilsObjectNameInfoEXT nameInfo = {};
		nameInfo.objectType = vkHandle.objectType;
		nameInfo.objectHandle = HandleTraits<HandleType>::CastToU64Handle(vkHandle);
		const std::string objectName = std::string(name) + std::string(HandleTraits<HandleType>::GetTypePostFix());
		nameInfo.pObjectName = objectName.c_str();
		CHECK_VULKAN_HPP(mVkDevice.setDebugUtilsObjectNameEXT(nameInfo));
#endif // WITH_DEBUG_RENDERING_FEATURES
	}
} // Turbo
