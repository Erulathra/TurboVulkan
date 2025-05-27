#pragma once

#include "RHICore.h"

#include "QueueFamilyIndices.h"

namespace Turbo
{
	class HardwareDevice;
	struct QueueFamilyIndices;

	class Device
	{
	public:
		struct AcquiredQueues
		{
			VkQueue GraphicsQueue = nullptr;
			VkQueue PresentQueue = nullptr;

			[[nodiscard]] bool IsValid() const;
		};

	public:
		void Init(const HardwareDevice* InHWDevice);
		void Destroy();

		[[nodiscard]] bool IsValid() const;

	public:
		[[nodiscard]] VkDevice GetVulkanDevice() const { return VulkanDevice; }
		[[nodiscard]] const QueueFamilyIndices& GetQueueIndices() const {return QueueIndices; }

	private:
		void SetupQueues();
		static VkPhysicalDeviceFeatures GetRequiredDeviceFeatures();

	private:
		VkDevice VulkanDevice = nullptr;

		QueueFamilyIndices QueueIndices;
		AcquiredQueues Queues;
	};
}
