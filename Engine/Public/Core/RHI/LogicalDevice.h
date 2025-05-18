#pragma once

#include "RHICore.h"

#include "QueueFamilyIndices.h"

namespace Turbo
{
	struct QueueFamilyIndices;


	class LogicalDevice
	{
	public:
		struct AcquiredQueues
		{
			VkQueue GraphicsQueue = nullptr;
			VkQueue PresentQueue = nullptr;

			[[nodiscard]] bool IsValid() const;
		};

	public:
		void Init(const HardwareDevicePtr& InHWDevice);
		void Destroy();

		[[nodiscard]] bool IsValid() const;

	public:
		[[nodiscard]] const VkDevice GetVulkanDevice() const { return VulkanDevice; }
		[[nodiscard]] const HardwareDevice* GetHardwareDevice() const { return HWDevice.lock().get(); }
		[[nodiscard]] const QueueFamilyIndices& GetQueueIndices() const {return QueueIndices; }

	private:
		void SetupQueues();
		static VkPhysicalDeviceFeatures GetRequiredDeviceFeatures();

	private:
		VkDevice VulkanDevice = nullptr;

		HardwareDeviceWeakPtr HWDevice;
		QueueFamilyIndices QueueIndices;
		AcquiredQueues Queues;
	};
}
