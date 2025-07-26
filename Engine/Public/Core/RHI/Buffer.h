#pragma once

#include "Core/RHI/RHICore.h"

namespace Turbo
{
	class FVulkanDevice;
	class FRHIDestroyQueue;

	class FBuffer
	{
		GENERATED_BODY(FBuffer)
	private:
		explicit FBuffer(FVulkanDevice* device);

	public:
		FBuffer() = delete;
		FBuffer(FBuffer& other) = delete;
		FBuffer& operator=(const FBuffer& other) = delete;
		FBuffer(const FBuffer& other) = delete;

	public:
		static std::unique_ptr<FBuffer> CreateUnique(FVulkanDevice* device, size_t allocSize, vk::BufferUsageFlags usageFlags, vma::MemoryUsage memoryUsageFlags);
		static std::shared_ptr<FBuffer> CreateShared(FVulkanDevice* device, size_t allocSize, vk::BufferUsageFlags usageFlags, vma::MemoryUsage memoryUsageFlags);
		~FBuffer();

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
