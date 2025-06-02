#pragma once

#include "RHICore.h"

namespace Turbo
{
	class FVulkanHardwareDevice;
	class FVulkanDevice;

	class FSwapChain
	{
	public:

	public:
		FSwapChain() = delete;
		explicit FSwapChain(FVulkanDevice& device);

		void Init();
		void Destroy();

	private:
		[[nodiscard]] vk::SurfaceFormatKHR SelectBestSurfacePixelFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats) const;
		[[nodiscard]] vk::PresentModeKHR SelectBestPresentMode(const std::vector<vk::PresentModeKHR>& availableModes) const;
		[[nodiscard]] vk::Extent2D CalculateSwapChainExtent(const vk::SurfaceCapabilitiesKHR& capabilities) const;

	private:
		void InitializeImageViews();

	private:
		FVulkanDevice* mDevice = nullptr;

		vk::SwapchainKHR mVulkanSwapChain = nullptr;

		vk::Format mImageFormat = vk::Format::eUndefined;
		vk::Extent2D mImageSize{};

		std::vector<vk::Image> mImages;
		std::vector<vk::ImageView> mImageViews;
	};
}
