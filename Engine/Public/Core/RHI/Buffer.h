#pragma once

#include "Core/RHI/RHICore.h"

namespace Turbo
{
	class FVulkanDevice;
	class FRHIDestroyQueue;

	class FVulkanBuffer
	{
		GENERATED_BODY(FVulkanBuffer)
	private:
		explicit FVulkanBuffer(FVulkanDevice* device);

	public:
		FVulkanBuffer() = delete;
		FVulkanBuffer(FVulkanBuffer& other) = delete;
		FVulkanBuffer& operator=(const FVulkanBuffer& other) = delete;
		FVulkanBuffer(const FVulkanBuffer& other) = delete;

	public:
		static std::unique_ptr<FVulkanBuffer> CreateUnique(FVulkanDevice* device, size_t allocSize, vk::BufferUsageFlags usageFlags, vma::MemoryUsage memoryUsageFlags);
		static std::shared_ptr<FVulkanBuffer> CreateShared(FVulkanDevice* device, size_t allocSize, vk::BufferUsageFlags usageFlags, vma::MemoryUsage memoryUsageFlags);
		~FVulkanBuffer();

		void RequestDestroy(FRHIDestroyQueue& deletionQueue);
		void Destroy();

	public:
		[[nodiscard]] vk::Buffer GetBuffer() const { return mBuffer; }
		[[nodiscard]] vma::Allocation GetAllocation() const { return mAllocation; }
		[[nodiscard]] byte* GetMappedData() const { return mMappedDataPtr; }

	private:
		void InitResource(size_t allocSize, vk::BufferUsageFlags usageFlags, vma::MemoryUsage memoryUsageFlags);

	private:
		FVulkanDevice* mDevice;

		vk::Buffer mBuffer = nullptr;
		vma::Allocation mAllocation = nullptr;
		byte* mMappedDataPtr = nullptr;

		bool bManualDestroy = false;

	public:
		friend class FBufferDestroyer;
	};
} // Turbo
