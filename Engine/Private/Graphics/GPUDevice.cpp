#include "Graphics/GPUDevice.h"

#include "VkBootstrap.h"
#include "Core/Engine.h"

#include "Core/Window.h"

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

namespace Turbo
{
	void FGPUDevice::Init(const FGPUDeviceBuilder& gpuDeviceBuilder)
	{
		TURBO_LOG(LOG_GPU_DEVICE, Info, "Initializing GPU Device.");

		gpuDeviceBuilder.mWindow->InitForVulkan();
		gpuDeviceBuilder.mWindow->Init();

		std::vector<cstring> instanceRequiredExtensions = gpuDeviceBuilder.mWindow->GetVulkanRequiredExtensions();

		VULKAN_HPP_DEFAULT_DISPATCHER.init();
		const vkb::Instance builtInstance = CreateVkInstance(instanceRequiredExtensions);
		VULKAN_HPP_DEFAULT_DISPATCHER.init(mVkInstance);

		TURBO_CHECK(gpuDeviceBuilder.mWindow->CreateVulkanSurface(mVkInstance));
		mVkWindowSurface = gpuDeviceBuilder.mWindow->GetVulkanSurface();

		const vkb::PhysicalDevice selectedPhysicalDevice = SelectPhysicalDevice(builtInstance);
		const vkb::Device device = CreateDevice(selectedPhysicalDevice);
		VULKAN_HPP_DEFAULT_DISPATCHER.init(mVkDevice);
		CreateSwapchain();
		CreateVulkanMemoryAllocator();
		CreateFrameDatas();
	}

	vkb::Instance FGPUDevice::CreateVkInstance(const std::vector<cstring>& requiredExtensions)
	{
		vkb::InstanceBuilder instanceBuilder;
		instanceBuilder
			.set_app_name("TurboEngine")
			.set_app_version(TURBO_VERSION())
			.enable_extensions(requiredExtensions)
#if WITH_VALIDATION_LAYERS
			.request_validation_layers(true)
			.set_debug_callback(&ThisClass::ValidationLayerCallback)
			// .set_debug_messenger_severity(VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
#endif // WITH_VALIDATION_LAYERS
			.require_api_version(1, 3, 0);

		vkb::Result<vkb::Instance> buildInstanceResult = instanceBuilder.build();
		TURBO_CHECK_MSG(buildInstanceResult, "Vulkan Instance Creation failed. Reason: {}", buildInstanceResult.error().message())

		mVkInstance = buildInstanceResult.value();
		mVkDebugUtilsMessenger = buildInstanceResult.value().debug_messenger;

		return buildInstanceResult.value();
	}

	vkb::PhysicalDevice FGPUDevice::SelectPhysicalDevice(const vkb::Instance& builtInstance)
	{
		TURBO_CHECK(mVkWindowSurface)

		vk::PhysicalDeviceFeatures deviceFeatures = {};
		deviceFeatures.setTextureCompressionBC(true);

		vk::PhysicalDeviceVulkan12Features device12Features = {};
		device12Features.setBufferDeviceAddress(true);
		device12Features.setDescriptorIndexing(true);
		device12Features.setDescriptorBindingPartiallyBound(true);
		device12Features.setRuntimeDescriptorArray(true);

		vk::PhysicalDeviceVulkan13Features device13Features = {};
		device13Features.setDynamicRendering(true);
		device13Features.setSynchronization2(true);

		vkb::PhysicalDeviceSelector physicalDeviceSelector(builtInstance);
		physicalDeviceSelector
			.set_surface(mVkWindowSurface)
			.require_present(true)
			.set_required_features(deviceFeatures)
			.set_required_features_12(device12Features)
			.set_required_features_13(device13Features)
			.set_minimum_version(1, 3)
			.prefer_gpu_device_type(vkb::PreferredDeviceType::discrete)
			.add_required_extension(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

		vkb::Result<vkb::PhysicalDevice> selectPhysicalDeviceResult = physicalDeviceSelector.select();
		TURBO_CHECK_MSG(selectPhysicalDeviceResult, "Physical device selection failed. Reason: {}", selectPhysicalDeviceResult.error().message())

		vkb::PhysicalDevice physicalDevice = selectPhysicalDeviceResult.value();

		mVkPhysicalDevice = physicalDevice;
		TURBO_LOG(LOG_GPU_DEVICE, Info, "Selected {} as primary physical device.", physicalDevice.name);

		return physicalDevice;
	}

	vkb::Device FGPUDevice::CreateDevice(const vkb::PhysicalDevice& physicalDevice)
	{
		vkb::DeviceBuilder deviceBuilder(physicalDevice);
		vkb::Result<vkb::Device> buildDeviceResult = deviceBuilder.build();
		TURBO_CHECK_MSG(buildDeviceResult, "Device creation failed. Reason: {}", buildDeviceResult.error().message())

		mVkDevice = buildDeviceResult.value();

		vkb::Result<VkQueue> getQueueResult = buildDeviceResult->get_queue(vkb::QueueType::graphics);
		TURBO_CHECK_MSG(getQueueResult, "Queue query failed. Reason: {}", getQueueResult.error().message())
		mVkQueue = getQueueResult.value();

		vkb::Result<uint32> getQueueIndexResult = buildDeviceResult->get_queue_index(vkb::QueueType::graphics);
		TURBO_CHECK_MSG(getQueueIndexResult, "Queue family query failed. Reason: {}", getQueueIndexResult.error().message())
		mVkQueueFamilyIndex = getQueueIndexResult.value();

		TURBO_CHECK_MSG(
			buildDeviceResult->get_queue_index(vkb::QueueType::graphics).value() == buildDeviceResult->get_queue_index(vkb::QueueType::present).value(),
			"Kurevsko nie dobre novinky."
			)

		return buildDeviceResult.value();
	}

	vkb::Swapchain FGPUDevice::CreateSwapchain()
	{
		TURBO_CHECK(mVkWindowSurface)

		const glm::ivec2& frameBufferSize = gEngine->GetWindow()->GetFrameBufferSize();
		TURBO_LOG(LOG_GPU_DEVICE, Info, "Creating swapchain of size: {}", frameBufferSize);

		vkb::SwapchainBuilder swapchainBuilder {mVkPhysicalDevice, mVkDevice, mVkWindowSurface};
		vkb::Result<vkb::Swapchain> buildSwapchainResult = swapchainBuilder
			.set_desired_present_mode(static_cast<VkPresentModeKHR>(GetBestPresentMode()))
			.set_desired_format(vk::SurfaceFormatKHR{ vk::Format::eB8G8R8A8Srgb, vk::ColorSpaceKHR::eSrgbNonlinear })
			.set_desired_extent(frameBufferSize.x, frameBufferSize.y)
			.add_image_usage_flags( static_cast<VkImageUsageFlags>(vk::ImageUsageFlagBits::eTransferDst))
			.build();

		TURBO_CHECK_MSG(buildSwapchainResult, "Cannot construct Swapchain. Reason: {}", buildSwapchainResult.error().message())

		vkb::Swapchain builtSwapchain = buildSwapchainResult.value();
		mVkSwapchain = builtSwapchain;
		mVkSurfaceFormat = { builtSwapchain.image_format, builtSwapchain.color_space };
		mPresentMode = static_cast<vk::PresentModeKHR>(builtSwapchain.present_mode);

		TURBO_LOG(LOG_GPU_DEVICE, Info, "Selected present mode: {}", magic_enum::enum_name(mPresentMode));

		const std::vector<VkImage> builtImages = builtSwapchain.get_images().value();
		const std::vector<VkImageView> builtImageViews = builtSwapchain.get_image_views().value();

		vk::SemaphoreCreateInfo semaphoreCreateInfo = VulkanInitializers::SemaphoreCreateInfo();

		mNumSwapChainImages = builtSwapchain.image_count;
		for (uint32 imageId = 0; imageId < mNumSwapChainImages; ++imageId)
		{
			FTextureHandle handle = mTexturePool->Acquire();
			FTexture* texture = mTexturePool->Access(handle);
			texture->mImage = builtImages[imageId];
			texture->mImageView = builtImageViews[imageId];
			texture->mFormat = mVkSurfaceFormat.format;
			texture->mAspectFlags = vk::ImageAspectFlagBits::eColor;

			texture->mWidth = frameBufferSize.x;
			texture->mHeight = frameBufferSize.y;

			texture->mHandle = handle;

			static const std::array<FName, 3> swapChainTextureNames = {FName("SwapchainTexture_1"), FName("SwapchainTexture_2"), FName("SwapchainTexture_3")};
			texture->mName = swapChainTextureNames[imageId];

			mSwapChainTextures[imageId] = handle;

			CHECK_VULKAN_RESULT(mSubmitSemaphores[imageId], mVkDevice.createSemaphore(semaphoreCreateInfo));
		}

		return builtSwapchain;
	}

	void FGPUDevice::CreateVulkanMemoryAllocator()
	{
		vma::AllocatorCreateInfo createInfo = {};
		createInfo.setPhysicalDevice(mVkPhysicalDevice);
		createInfo.setDevice(mVkDevice);
		createInfo.setInstance(mVkInstance);

		createInfo.flags = vma::AllocatorCreateFlagBits::eBufferDeviceAddress;

		vma::VulkanFunctions vulkanFunctions{};
		vulkanFunctions.setVkGetInstanceProcAddr(VULKAN_HPP_DEFAULT_DISPATCHER.vkGetInstanceProcAddr);
		vulkanFunctions.setVkGetDeviceProcAddr(VULKAN_HPP_DEFAULT_DISPATCHER.vkGetDeviceProcAddr);

		createInfo.pVulkanFunctions = &vulkanFunctions;

		CHECK_VULKAN_RESULT(mVmaAllocator, vma::createAllocator(createInfo));
	}

	void FGPUDevice::CreateFrameDatas()
	{
		TURBO_LOG(LOG_GPU_DEVICE, Info, "Creating frames data")

		const vk::FenceCreateInfo fenceCreateInfo = VulkanInitializers::FenceCreateInfo(vk::FenceCreateFlagBits::eSignaled);
		const vk::SemaphoreCreateInfo semaphoreCreateInfo = VulkanInitializers::SemaphoreCreateInfo();

		for (FBufferedFrameData& frameData : mFrameDatas)
		{
			CHECK_VULKAN_RESULT(frameData.mCommandBufferExecutedFence, mVkDevice.createFence(fenceCreateInfo));
			CHECK_VULKAN_RESULT(frameData.mImageAcquiredSemaphore, mVkDevice.createSemaphore(semaphoreCreateInfo));

			frameData.mCommandPool = CreateCommandPool();
			frameData.mCommandBuffer = CreateCommandBuffer(frameData.mCommandPool);
		}
	}

	vk::CommandPool FGPUDevice::CreateCommandPool()
	{
		vk::CommandPoolCreateInfo createInfo = {};
		createInfo.setQueueFamilyIndex(mVkQueueFamilyIndex);

		vk::CommandPool pool;
		CHECK_VULKAN_RESULT(pool, mVkDevice.createCommandPool(createInfo));

		return pool;
	}

	std::unique_ptr<FCommandBuffer> FGPUDevice::CreateCommandBuffer(vk::CommandPool commandPool)
	{
		vk::CommandBufferAllocateInfo allocateInfo = {};
		allocateInfo.setCommandPool(commandPool);
		allocateInfo.setCommandBufferCount(1);
		allocateInfo.setLevel(vk::CommandBufferLevel::ePrimary);

		std::vector<vk::CommandBuffer> commandBuffers;
		CHECK_VULKAN_RESULT(commandBuffers, mVkDevice.allocateCommandBuffers(allocateInfo));

		std::unique_ptr<FCommandBuffer> result = std::make_unique<FCommandBuffer>();
		result->mDevice = this;
		result->mVkCommandBuffer = commandBuffers.front();
		result->Reset();

		return result;
	}

	vk::PresentModeKHR FGPUDevice::GetBestPresentMode()
	{
		TURBO_CHECK(mVkWindowSurface)

		std::vector<vk::PresentModeKHR> supportedModes;
		CHECK_VULKAN_RESULT(supportedModes, mVkPhysicalDevice.getSurfacePresentModesKHR(mVkWindowSurface));

		if (mbVSync)
		{
			if (std::ranges::find(supportedModes, vk::PresentModeKHR::eImmediate) != supportedModes.end())
			{
				return vk::PresentModeKHR::eImmediate;
			}
		}

		if (std::ranges::find(supportedModes, vk::PresentModeKHR::eMailbox) != supportedModes.end())
		{
			return vk::PresentModeKHR::eMailbox;
		}

		return vk::PresentModeKHR::eFifo;
	}

	void FGPUDevice::Shutdown()
	{
		CHECK_VULKAN_HPP(mVkDevice.waitIdle())

		TURBO_LOG(LOG_GPU_DEVICE, Info, "Starting Gpu Device shutdown.")

		DestroyFrameDatas();
		DestroySwapChain();

		if (mVmaAllocator)
		{
			mVmaAllocator.destroy();
		}

		if (mVkDevice)
		{
			mVkDevice.destroy();
		}

		gEngine->GetWindow()->DestroyVulkanSurface(mVkInstance);

		if (mVkInstance)
		{
			mVkInstance.destroyDebugUtilsMessengerEXT(mVkDebugUtilsMessenger);
			mVkInstance.destroy();
		}
	}

	void FGPUDevice::BeginFrame()
	{
		const FBufferedFrameData& frameData = mFrameDatas[mBufferedFrameIndex];

		// Wait for previous frame fence, and reset it
		const vk::Fence renderCompleteFence = frameData.mCommandBufferExecutedFence;
		CHECK_VULKAN_HPP(mVkDevice.waitForFences({renderCompleteFence}, vk::True, kMaxTimeout));
		CHECK_VULKAN_HPP(mVkDevice.resetFences({renderCompleteFence}));

		// Acquire next swapchain image
		const vk::Semaphore imageAcquiredSemaphore = frameData.mImageAcquiredSemaphore;

		vk::Result acquireImageResult;
		std::tie(acquireImageResult, mCurrentSwapchainImageIndex) =
			mVkDevice.acquireNextImageKHR(mVkSwapchain, kMaxTimeout, imageAcquiredSemaphore, nullptr);

		if (acquireImageResult == vk::Result::eErrorOutOfDateKHR)
		{
			// TODO: resize swapchain
			TURBO_UNINPLEMENTED_MSG("resize swapchain");
		}

		CHECK_VULKAN_HPP(mVkDevice.resetCommandPool(frameData.mCommandPool));
		frameData.mCommandBuffer->Reset();
		frameData.mCommandBuffer->Begin();
	}

	void FGPUDevice::PresentFrame()
	{
		const FBufferedFrameData& frameData = mFrameDatas[mBufferedFrameIndex];

		const FTextureHandle swapChainTexture = mSwapChainTextures[mCurrentSwapchainImageIndex];
		frameData.mCommandBuffer->TransitionImage(swapChainTexture, vk::ImageLayout::ePresentSrcKHR);

		frameData.mCommandBuffer->End();

		// Submit command buffer
		const vk::Semaphore submitSemaphore = mSubmitSemaphores[mCurrentSwapchainImageIndex];

		const vk::CommandBufferSubmitInfo bufferSubmitInfo = frameData.mCommandBuffer->CreateSubmitInfo();
		const vk::SemaphoreSubmitInfo waitSemaphore = VkInit::SemaphoreSubmitInfo(frameData.mImageAcquiredSemaphore, vk::PipelineStageFlagBits2::eColorAttachmentOutput);
		const vk::SemaphoreSubmitInfo signalSemaphore = VkInit::SemaphoreSubmitInfo(submitSemaphore, vk::PipelineStageFlagBits2::eAllGraphics);
		const vk::SubmitInfo2 submitInfo = VkInit::SubmitInfo(bufferSubmitInfo, &signalSemaphore, &waitSemaphore);
		CHECK_VULKAN_HPP(mVkQueue.submit2(1, &submitInfo, frameData.mCommandBufferExecutedFence));

		// Present swapchain texture
		const vk::PresentInfoKHR presentInfo = VkInit::PresentInfo(mVkSwapchain, submitSemaphore, mCurrentSwapchainImageIndex);
		const vk::Result presentResult = mVkQueue.presentKHR(&presentInfo);

		if (presentResult == vk::Result::eErrorOutOfDateKHR)
		{
			// TODO: resize swapchain
			TURBO_UNINPLEMENTED_MSG("resize swapchain");
		}
		else
		{
			CHECK_VULKAN_HPP_MSG(presentResult, "Cannot present swapchain image.");
		}
	}

	void FGPUDevice::DestroySwapChain()
	{
		mVkDevice.destroySwapchainKHR(mVkSwapchain);

		for (uint32 imageId = 0; imageId < mNumSwapChainImages; ++imageId)
		{
			FTexture* texture = AccessTexture(mSwapChainTextures[imageId]);
			mVkDevice.destroyImageView(texture->mImageView);
			mTexturePool->Release(mSwapChainTextures[imageId]);

			mVkDevice.destroySemaphore(mSubmitSemaphores[imageId]);
		}

		for (uint32 imageId = 0; imageId < kMaxSwapChainImages; ++imageId)
		{
			mSwapChainTextures[imageId].Reset();
		}

		mNumSwapChainImages = 0;
	}

	void FGPUDevice::DestroyFrameDatas()
	{
		TURBO_LOG(LOG_GPU_DEVICE, Info, "Destroying frames data")

		for (FBufferedFrameData& frameData : mFrameDatas)
		{
			if (frameData.mCommandBufferExecutedFence)
			{
				mVkDevice.destroyFence(frameData.mCommandBufferExecutedFence);
				frameData.mCommandBufferExecutedFence = nullptr;
			}

			if (frameData.mImageAcquiredSemaphore)
			{
				mVkDevice.destroySemaphore(frameData.mImageAcquiredSemaphore);
				frameData.mImageAcquiredSemaphore = nullptr;
			}

			if (frameData.mCommandPool)
			{
				mVkDevice.destroyCommandPool(frameData.mCommandPool);
			}
		}
	}

	void FGPUDevice::AdvanceFrameCounters()
	{
		TURBO_CHECK(mNumSwapChainImages > 0)
		mBufferedFrameIndex = (mBufferedFrameIndex + 1) % kMaxBufferedFrames;

		++mRenderedFrames;
	}

	VkBool32 FGPUDevice::ValidationLayerCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
		void* userData
		)
	{
		if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
		{
			TURBO_LOG(LOG_RHI, Display, "{}", callbackData->pMessage)
		}
		else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
		{
			// Messages with info severity are very verbose, so I reduced its verbosity to display.
			TURBO_LOG(LOG_RHI, Display, "{}", callbackData->pMessage)
		}
		else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
		{
			TURBO_LOG(LOG_RHI, Warn, "{}", callbackData->pMessage)
		}
		else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
		{
			TURBO_LOG(LOG_RHI, Error, "{}", callbackData->pMessage)
			TURBO_DEBUG_BREAK();
		}

		return VK_FALSE;
	}
} // Turbo