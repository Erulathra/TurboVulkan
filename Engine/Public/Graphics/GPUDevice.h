#pragma once

#include "CommandBuffer.h"
#include "VkBootstrap.h"

#include "ResourceBuilders.h"
#include "DestoryQueue.h"
#include "Core/DataStructures/ResourcePool.h"
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

		std::unique_ptr<FCommandBuffer> mCommandBuffer;
		FDescriptorPoolHandle mDescriptorPoolHandle;

		FDestroyQueue mDestroyQueue;

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
		bool BeginFrame();
		bool PresentFrame();

		FCommandBuffer* GetCommandBuffer() { return mFrameDatas[mBufferedFrameIndex].mCommandBuffer.get(); }
		FDescriptorPoolHandle GetDescriptorPool() { return mFrameDatas[mBufferedFrameIndex].mDescriptorPoolHandle; }

		FTextureHandle GetPresentImage() { return mSwapChainTextures[mCurrentSwapchainImageIndex]; }

		void WaitIdle() const;

		void ImmediateSubmit(const FOnImmediateSubmit& immediateSubmitDelegate);

		/** Rendering interface end */

		/** Resource accessors */
	public:
		[[nodiscard]] FBuffer* AccessBuffer(FBufferHandle handle) { return mBufferPool->Access(handle); }
		[[nodiscard]] FTexture* AccessTexture(FTextureHandle handle) { return mTexturePool->Access(handle); }
		[[nodiscard]] FSampler* AccessSampler(FSamplerHandle handle) { return mSamplerPool->Access(handle); }
		[[nodiscard]] FPipeline* AccessPipeline(FPipelineHandle handle) { return mPipelinePool->Access(handle); }
		[[nodiscard]] FDescriptorPool* AccessDescriptorPool(FDescriptorPoolHandle handle) { return mDescriptorPoolPool->Access(handle); }
		[[nodiscard]] FDescriptorSetLayout* AccessDescriptorSetLayout(FDescriptorSetLayoutHandle handle) { return mDescriptorSetLayoutPool->Access(handle); }
		[[nodiscard]] FDescriptorSet* AccessDescriptorSet(FDescriptorSetHandle handle) { return mDescriptorSetPool->Access(handle); }
		[[nodiscard]] FShaderState* AccessShaderState(FShaderStateHandle handle) { return mShaderStatePool->Access(handle); }

		[[nodiscard]] const FBuffer* AccessBuffer(FBufferHandle handle) const { return mBufferPool->Access(handle); }
		[[nodiscard]] const FTexture* AccessTexture(FTextureHandle handle) const { return mTexturePool->Access(handle); }
		[[nodiscard]] const FSampler* AccessSampler(FSamplerHandle handle) const { return mSamplerPool->Access(handle); }
		[[nodiscard]] const FPipeline* AccessPipeline(FPipelineHandle handle) const { return mPipelinePool->Access(handle); }
		[[nodiscard]] const FDescriptorPool* AccessDescriptorPool(FDescriptorPoolHandle handle) const { return mDescriptorPoolPool->Access(handle); }
		[[nodiscard]] const FDescriptorSetLayout* AccessDescriptorSetLayout(FDescriptorSetLayoutHandle handle) const { return mDescriptorSetLayoutPool->Access(handle); }
		[[nodiscard]] const FDescriptorSet* AccessDescriptorSet(FDescriptorSetHandle handle) const { return mDescriptorSetPool->Access(handle); }
		[[nodiscard]] const FShaderState* AccessShaderState(FShaderStateHandle handle) const { return mShaderStatePool->Access(handle); }

		/** Resource accessors end */

		/** Resource creation */
	public:
		FBufferHandle CreateBuffer(const FBufferBuilder& builder, FCommandBuffer* commandBuffer = nullptr);
		FTextureHandle CreateTexture(const FTextureBuilder& builder);
		// FSamplerHandle CreateSampler();
		FPipelineHandle CreatePipeline(const FPipelineBuilder& builder);
		FDescriptorPoolHandle CreateDescriptorPool(const FDescriptorPoolBuilder& builder);
		FDescriptorSetLayoutHandle CreateDescriptorSetLayout(const FDescriptorSetLayoutBuilder& builder);
		FDescriptorSetHandle CreateDescriptorSet(const FDescriptorSetBuilder& builder);
		FShaderStateHandle CreateShaderState(const FShaderStateBuilder& builder);

		/** Resource creation end */

		/** Other resource related methods */
	public:
		void ResetDescriptorPool(FDescriptorPoolHandle descriptorPoolHandle);
		/** Other resource related methods end */

		/** Resource destroy */
	public:
		void DestroyBuffer(FBufferHandle handle);
		void DestroyTexture(FTextureHandle handle);
		void DestroyPipeline(FPipelineHandle handle);
		void DestroyDescriptorPool(FDescriptorPoolHandle handle);
		void DestroyDescriptorSetLayout(FDescriptorSetLayoutHandle handle);
		void DestroyShaderState(FShaderStateHandle handle);

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

		/** Initialization methods */
	private:
		vkb::Instance CreateVkInstance(const std::vector<cstring>& requiredExtensions);
		vkb::PhysicalDevice SelectPhysicalDevice(const vkb::Instance& builtInstance);
		vkb::Device CreateDevice(const vkb::PhysicalDevice& physicalDevice);
		vkb::Swapchain CreateSwapchain();
		void CreateVulkanMemoryAllocator();
		void CreateFrameDatas();
		vk::CommandPool CreateCommandPool();
		std::unique_ptr<FCommandBuffer> CreateCommandBuffer(vk::CommandPool commandPool, FName name = FName());

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

		/** Rendering interface */
	private:
		void AdvanceFrameCounters();

		/** Rendering interface end */

		/** Creation helpers */
	private:
		void InitVulkanTexture(const FTextureBuilder& builder, FTextureHandle handle, FTexture* texture);

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
		std::unique_ptr<FCommandBuffer> mImmediateCommandsBuffer;

		/** Immediate commands end */

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
