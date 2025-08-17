#include "Core/RHI/Buffer.h"

#include "Core/Engine.h"
#include "Core/RHI/VulkanDevice.h"
#include "Core/RHI/VulkanRHI.h"

namespace Turbo {
	class FBufferDestroyer : IDestroyer
	{
	public:
		FBufferDestroyer() = delete;
		explicit FBufferDestroyer(const FVulkanBuffer& buffer)
			: mBuffer(buffer.mBuffer)
			, mAllocation(buffer.mAllocation)
		{}

		virtual ~FBufferDestroyer() = default;

	public:
		virtual void Destroy(void* context) override
		{
			const FVulkanDevice* device = static_cast<FVulkanDevice*>(context);

			if (mBuffer && mAllocation)
			{
				device->GetAllocator().destroyBuffer(mBuffer, mAllocation);
			}
		}

	private:
		vk::Buffer mBuffer;
		vma::Allocation mAllocation;
	};

	FVulkanBuffer::FVulkanBuffer(FVulkanDevice* device)
		: mDevice(device)
	{

	}

	void FVulkanBuffer::InitResource(size_t allocSize, vk::BufferUsageFlags usageFlags, vma::MemoryUsage memoryUsageFlags)
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

	std::unique_ptr<FVulkanBuffer> FVulkanBuffer::CreateUnique(FVulkanDevice* device, size_t allocSize, vk::BufferUsageFlags usageFlags, vma::MemoryUsage memoryUsageFlags)
	{
		std::unique_ptr<FVulkanBuffer> result(new FVulkanBuffer(device));
		result->InitResource(allocSize, usageFlags, memoryUsageFlags);

		return std::move(result);
	}

	std::shared_ptr<FVulkanBuffer> FVulkanBuffer::CreateShared(FVulkanDevice* device, size_t allocSize, vk::BufferUsageFlags usageFlags, vma::MemoryUsage memoryUsageFlags)
	{
		std::shared_ptr<FVulkanBuffer> result(new FVulkanBuffer(device));
		result->InitResource(allocSize, usageFlags, memoryUsageFlags);

		return std::move(result);
	}

	FVulkanBuffer::~FVulkanBuffer()
	{
		if (!bManualDestroy)
		{
			Destroy();
		}
	}

	void FVulkanBuffer::RequestDestroy(FDestroyQueue& deletionQueue)
	{
		bManualDestroy = true;
		deletionQueue.RequestDestroy(FBufferDestroyer(*this));
	}

	void FVulkanBuffer::Destroy()
	{
		bManualDestroy = true;

#if 0
		FDestroyQueue& deletionQueue = gEngine->GetRHI()->GetDeletionQueue();
		deletionQueue.RequestDestroy(FBufferDestroyer(*this));
#endif

		mBuffer = nullptr;
		mAllocation = nullptr;
		mMappedDataPtr = nullptr;

		mDevice->GetAllocator().destroyBuffer(mBuffer, mAllocation);
	}
} // Turbo