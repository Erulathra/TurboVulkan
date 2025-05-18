#pragma once

#include <volk.h>

namespace Turbo
{
	class HardwareDevice;
	using HardwareDevicePtr = std::shared_ptr<HardwareDevice>;
	using HardwareDeviceWeakPtr = std::weak_ptr<HardwareDevice>;

	class LogicalDevice;
	using LogicalDevicePtr = std::shared_ptr<LogicalDevice>;
	using LogicalDeviceWeakPtr = std::weak_ptr<LogicalDevice>;

	class SwapChain;
	using SwapChainPtr = std::shared_ptr<SwapChain>;
	using SwapChainWeakPtr = std::weak_ptr<SwapChain>;
}
