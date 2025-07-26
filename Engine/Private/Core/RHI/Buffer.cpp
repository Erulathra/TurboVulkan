#include "Core/RHI/Buffer.h"

#include "Core/Engine.h"
#include "Core/RHI/VulkanDevice.h"
#include "Core/RHI/VulkanRHI.h"

namespace Turbo {
	class FBufferDestroyer : IDestroyer
	{
	public:
		FBufferDestroyer() = delete;
		explicit FBufferDestroyer(const FBuffer& buffer)
			: mBuffer(buffer.mBuffer)
			, mAllocation(buffer.mAllocation)
		{}

		virtual ~FBufferDestroyer() = default;

	public:
		virtual void Destroy(const FVulkanDevice* device) override
		{
			if (mBuffer && mAllocation)
			{
				device->GetAllocator().destroyBuffer(mBuffer, mAllocation);
			}
		}

	private:
		vk::Buffer mBuffer;
		vma::Allocation mAllocation;
	};

	FBuffer::FBuffer(FVulkanDevice* device)
		: mDevice(device)
	{

	}

	void FBuffer::InitResource(size_t allocSize, vk::BufferUsageFlags usageFlags, vma::MemoryUsage memoryUsageFlags)
	{
		TURBO_CHECK(mDevice);

		vk::BufferCreateInfo bufferInfo{};
		bufferInfo.size = allocSize;
		bufferInfo.usage = usageFlags;

		vma::AllocationCreateInfo vmaAllocInfo{};
		vmaAllocInfo.usage = memoryUsageFlags;
		vmaAllocInfo.flags = vma::AllocationCreateFlagBits::eMapped | vma::AllocationCreateFlagBits::eHostAccessSequentialWrite;

		vma::AllocationInfo allocationInfo;
		std::pair<vk::Buffer, vma::Allocation> creationResult;
		CHECK_VULKAN_RESULT(creationResult, mDevice->GetAllocator().createBuffer(bufferInfo, vmaAllocInfo, &allocationInfo));
		std::tie(mBuffer, mAllocation) = creationResult;
		mMappedDataPtr = static_cast<byte*>(allocationInfo.pMappedData);
	}

	std::unique_ptr<FBuffer> FBuffer::CreateUnique(FVulkanDevice* device, size_t allocSize, vk::BufferUsageFlags usageFlags, vma::MemoryUsage memoryUsageFlags)
	{
		std::unique_ptr<FBuffer> result(new FBuffer(device));
		result->InitResource(allocSize, usageFlags, memoryUsageFlags);

		return std::move(result);
	}

	std::shared_ptr<FBuffer> FBuffer::CreateShared(FVulkanDevice* device, size_t allocSize, vk::BufferUsageFlags usageFlags, vma::MemoryUsage memoryUsageFlags)
	{
		std::shared_ptr<FBuffer> result(new FBuffer(device));
		result->InitResource(allocSize, usageFlags, memoryUsageFlags);

		return std::move(result);
	}

	FBuffer::~FBuffer()
	{
		if (!bManualDestroy)
		{
			Destroy();
		}
	}

	void FBuffer::RequestDestroy(FRHIDestroyQueue& deletionQueue)
	{
		bManualDestroy = true;
		deletionQueue.RequestDestroy(FBufferDestroyer(*this));
	}

	void FBuffer::Destroy()
	{
		bManualDestroy = true;

		FRHIDestroyQueue& deletionQueue = gEngine->GetRHI()->GetDeletionQueue();
		deletionQueue.RequestDestroy(FBufferDestroyer(*this));

		mBuffer = nullptr;
		mAllocation = nullptr;
		mMappedDataPtr = nullptr;

		mDevice->GetAllocator().destroyBuffer(mBuffer, mAllocation);
	}
} // Turbo