#pragma once

#include "RHICore.h"

namespace Turbo
{
	class FVulkanHardwareDevice;
	class FVulkanDevice;

	class FSwapChain
	{
	public:
		FSwapChain() = delete;
		explicit FSwapChain(FVulkanDevice& device);

	public:
		void Init();
		void Destroy();

		[[nodiscard]] bool IsValid() const;

	public:
		[[nodiscard]] vk::SwapchainKHR GetVulkanSwapChain() const { return mVulkanSwapChain; }
		[[nodiscard]] vk::Format GetImageFormat() const { return mImageFormat; }
		[[nodiscard]] vk::Extent2D GetImageSize() const { return mImageSize; }
		[[nodiscard]] int32 GetNumBufferedFrames() const { return mImages.size(); }
		[[nodiscard]] std::vector<vk::ImageView> GetImageViews() const { return mImageViews; }

		[[nodiscard]] const vk::Image& GetImage(uint32 id) const { return mImages[id]; }
		[[nodiscard]] const vk::ImageView& GetImageView(uint32 id) const { return mImageViews[id]; }

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
