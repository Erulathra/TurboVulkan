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
			VkQueue GraphicsQueue = nullptr;
			VkQueue PresentQueue = nullptr;

			[[nodiscard]] bool IsValid() const;
		};

	public:
		void Init(const FVulkanHardwareDevice* InHWDevice);
		void Destroy();

		[[nodiscard]] bool IsValid() const;

	public:
		[[nodiscard]] VkDevice GetVulkanDevice() const { return mVulkanDevice; }
		[[nodiscard]] const FQueueFamilyIndices& GetQueueIndices() const {return mQueueIndices; }

	private:
		void SetupQueues();
		static VkPhysicalDeviceFeatures GetRequiredDeviceFeatures();

	private:
		VkDevice mVulkanDevice = nullptr;

		FQueueFamilyIndices mQueueIndices;
		AcquiredQueues mQueues;
	};
}
