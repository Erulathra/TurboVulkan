#pragma once

#include "RHICore.h"

namespace Turbo
{
	class FVulkanHardwareDevice;
	class FVulkanDevice;

	struct FSwapchainImageData
	{
		vk::Image Image;
		vk::ImageView ImageView;

		vk::Semaphore QueueSubmitSemaphore;
	};

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
		void RequestResize() { bResizeRequested = true; };
		bool ResizeIfRequested();

		[[nodiscard]] vk::SwapchainKHR GetVulkanSwapChain() const { return mVulkanSwapChain; }
		[[nodiscard]] vk::Format GetImageFormat() const { return mImageFormat; }
		[[nodiscard]] glm::uvec2 GetImageSize() const { return {mImageSize.width, mImageSize.height}; }

		[[nodiscard]] const FSwapchainImageData& GetImage() const { return mImageDatas[mImageIndex]; }
		[[nodiscard]] int32 GetNumImages() const { return mImageDatas.size(); }

		bool AcquireImage(const vk::Semaphore& acquireSemaphore);
		void PresentImage();

	private:
		[[nodiscard]] vk::SurfaceFormatKHR SelectBestSurfacePixelFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats) const;
		[[nodiscard]] vk::PresentModeKHR SelectBestPresentMode(const std::vector<vk::PresentModeKHR>& availableModes) const;
		[[nodiscard]] vk::Extent2D CalculateSwapChainExtent(const vk::SurfaceCapabilitiesKHR& capabilities) const;

	private:
		void InitializeImageViews();
		void InitializeSemaphores();

	private:
		FVulkanDevice* mDevice = nullptr;

		vk::SwapchainKHR mVulkanSwapChain = nullptr;

		vk::Format mImageFormat = vk::Format::eUndefined;
		vk::Extent2D mImageSize{};

		std::vector<FSwapchainImageData> mImageDatas;
		uint32 mImageIndex = 0;

		bool bResizeRequested = false;
	};
}
