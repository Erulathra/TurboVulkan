#pragma once

#include "RHICore.h"

#include "QueueFamilyIndices.h"

namespace Turbo
{
	class FVulkanHardwareDevice;
	struct FQueueFamilyIndices;

	class FVulkanDevice
	{
	public:
		struct AcquiredQueues
		{
			vk::Queue GraphicsQueue = nullptr;
			vk::Queue PresentQueue = nullptr;
			vk::Queue TransferQueue = nullptr;

			[[nodiscard]] bool IsValid() const;
		};

	public:
		FVulkanDevice() = delete;
		explicit FVulkanDevice(FVulkanHardwareDevice& hardwareDevice);

		void Init();
		void Destroy();

		[[nodiscard]] bool IsValid() const;

	public:
		[[nodiscard]] vk::Device Get() const { return mVulkanDevice; }
		[[nodiscard]] vk::CommandPool GetRenderCommandPool() const { return mRenderCommandPool; }

		[[nodiscard]] FVulkanHardwareDevice* GetHardwareDevice() const { return mHardwareDevice; }
		[[nodiscard]] const FQueueFamilyIndices& GetQueueIndices() const {return mQueueIndices; }

		[[nodiscard]] const AcquiredQueues& GetQueues() const { return mQueues; }

	private:
		void SetupQueues();
		vk::PhysicalDeviceFeatures GetRequiredDeviceFeatures();

		void SetupCommandPools();

	private:
		FVulkanHardwareDevice* mHardwareDevice = nullptr;

		vk::Device mVulkanDevice = nullptr;
		vk::CommandPool mRenderCommandPool = nullptr;

		FQueueFamilyIndices mQueueIndices;
		AcquiredQueues mQueues;
	};
}
