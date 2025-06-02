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
		[[nodiscard]] FVulkanHardwareDevice* GetHardwareDevice() const { return mHardwareDevice; }
		[[nodiscard]] vk::Device& GetVulkanDevice() { return mVulkanDevice; }
		[[nodiscard]] const FQueueFamilyIndices& GetQueueIndices() const {return mQueueIndices; }

	private:
		void SetupQueues();
		vk::PhysicalDeviceFeatures GetRequiredDeviceFeatures();

	private:
		FVulkanHardwareDevice* mHardwareDevice = nullptr;

		vk::Device mVulkanDevice = nullptr;

		FQueueFamilyIndices mQueueIndices;
		AcquiredQueues mQueues;
	};
}
