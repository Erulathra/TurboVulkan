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
		[[nodiscard]] VkDevice GetVulkanDevice() const { return VulkanDevice; }
		[[nodiscard]] const FQueueFamilyIndices& GetQueueIndices() const {return QueueIndices; }

	private:
		void SetupQueues();
		static VkPhysicalDeviceFeatures GetRequiredDeviceFeatures();

	private:
		VkDevice VulkanDevice = nullptr;

		FQueueFamilyIndices QueueIndices;
		AcquiredQueues Queues;
	};
}
