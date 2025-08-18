#pragma once

#include "ResourceBuilders.h"
#include "Core/DataStructures/DestoryQueue.h"
#include "Core/DataStructures/ResourcePool.h"
#include "Graphics/Resources.h"
#include "Graphics/ResourceBuilders.h"

namespace Turbo
{
	class FWindow;

	class FGPUDevice
	{
		GENERATED_BODY(FGPUDevice)

	public:
		void Init(const FGPUDeviceBuilder& gpuDeviceBuilder);
		void Shutdown();

	private:
		static VkBool32 ValidationLayerCallback(
			VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageType,
			const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
			void* userData
		);

	private:
		/** Resource pools */
		TResourcePool<FBuffer, FBufferHandle, 4096> mBufferPool;
		TResourcePool<FTexture, FTextureHandle, 512> mTexturePool;
		TResourcePool<FSampler, FSamplerHandle, 32> mSamplerPool;
		TResourcePool<FPipeline, FPipelineHandle, 128> mPipelinePool;
		TResourcePool<FDescriptorSetLayout, FDescriptorSetLayoutHandle, 128> mDescriptorSetLayoutPool;
		TResourcePool<FDescriptorSet, FDescriptorSetHandle, 256> mDescriptorSetPool;
		TResourcePool<FShaderState, FShaderStateHandle, 128> mShaderStatePool;

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
		std::array<vk::Image, kMaxSwapChainImages> mSwapChainImages;
		std::array<vk::ImageView, kMaxSwapChainImages> mSwapChainImageViews;
		uint32 mNumSwapChainImages = 0;

		// Per frame synchronization
		std::array<vk::Semaphore, kMaxSwapChainImages> mRenderCompleteSemaphores;
		std::array<vk::Semaphore, kMaxSwapChainImages> mImageAcquiredSemaphores;
		std::array<vk::Fence, kMaxSwapChainImages> mCommandBufferExecutedFences;

		/** Swapchain end */

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
