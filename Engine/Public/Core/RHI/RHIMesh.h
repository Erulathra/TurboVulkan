#pragma once

#include "Buffer.h"
#include "VulkanDevice.h"
#include "VulkanRHI.h"
#include "Core/Engine.h"
#include "Core/RHI/RHICore.h"
#include "Core/RHI/MeshHelpers.h"

namespace Turbo
{
	class FRHIMesh
	{
		GENERATED_BODY(FRHIMesh)

	private:
		FRHIMesh() = default;

	public:
		template<typename VertexType>
		static std::shared_ptr<FRHIMesh> CreateShared(FVulkanDevice* device, std::span<uint32> indices, std::span<VertexType> vertices);
		template<typename VertexType>
		static std::unique_ptr<FRHIMesh> CreateUnique(FVulkanDevice* device, std::span<uint32> indices, std::span<VertexType> vertices);

	public:
		void Draw(const vk::CommandBuffer cmd, uint32 instanceCount = 1);

	public:
		[[nodiscard]] std::shared_ptr<FBuffer> GetVertexBuffer() const { return mVertexBuffer; }
		[[nodiscard]] std::shared_ptr<FBuffer> GetIndexBuffer() const { return mIndexBuffer; }
		[[nodiscard]] vk::DeviceAddress GetVertexBufferAddress() const { return mVertexBufferAddress; }

	private:
		template<typename VertexType>
		void Init(FVulkanDevice* device, std::span<uint32> indices, std::span<VertexType> vertices);

	private:
		std::shared_ptr<FBuffer> mVertexBuffer = nullptr;
		std::shared_ptr<FBuffer> mIndexBuffer = nullptr;

		vk::DeviceAddress mVertexBufferAddress = 0;

		size_t mVertexBufferSize = 0;
		size_t mIndexBufferSize = 0;
	};

	template <typename VertexType>
	std::shared_ptr<FRHIMesh> FRHIMesh::CreateShared(FVulkanDevice* device, std::span<uint32> indices, std::span<VertexType> vertices)
	{
		std::shared_ptr<FRHIMesh> result(new FRHIMesh());
		result->Init<VertexType>(device, indices, vertices);

		return result;
	}

	template <typename VertexType>
	std::unique_ptr<FRHIMesh> FRHIMesh::CreateUnique(FVulkanDevice* device, std::span<uint32> indices, std::span<VertexType> vertices)
	{
		std::unique_ptr<FRHIMesh> result(new FRHIMesh());
		result->Init<VertexType>(device, indices, vertices);

		return result;
	}

	template <typename VertexType>
	void FRHIMesh::Init(FVulkanDevice* device, std::span<uint32> indices, std::span<VertexType> vertices)
	{
		TRACE_ZONE_SCOPED();
		TURBO_CHECK(device);

		mVertexBufferSize = vertices.size() * sizeof(VertexType);
		mIndexBufferSize = indices.size() * sizeof(uint32);

		mVertexBuffer = FBuffer::CreateShared(
			device,
			mVertexBufferSize,
			vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eShaderDeviceAddress,
			vma::MemoryUsage::eAutoPreferDevice
			);

		mIndexBuffer = FBuffer::CreateShared(
			device,
			mIndexBufferSize,
			vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst,
			vma::MemoryUsage::eAutoPreferDevice
			);

		mVertexBufferAddress = device->GetBufferAddress(mVertexBuffer.get());

		std::unique_ptr<FBuffer> tempBuffer = FBuffer::CreateUnique(
			device,
			mVertexBufferSize + mIndexBufferSize,
			vk::BufferUsageFlagBits::eTransferSrc,
			vma::MemoryUsage::eAutoPreferHost
			);

		// Copy from vectors to buffer
		byte* tempBufferData = tempBuffer->GetMappedData();
		std::memcpy(tempBufferData, vertices.data(), mVertexBufferSize);
		std::memcpy(tempBufferData + mVertexBufferSize, indices.data(), mIndexBufferSize + 8);

		// Copy from host to device
		gEngine->GetRHI()->SubmitImmediateCommand(
			FSubmitDelegate::CreateLambda([&](vk::CommandBuffer cmd)
			{
				vk::BufferCopy verticesCopy{};
				verticesCopy.size = mVertexBufferSize;
				cmd.copyBuffer(tempBuffer->GetBuffer(), mVertexBuffer->GetBuffer(), 1, &verticesCopy);

				vk::BufferCopy indicesCopy{};
				indicesCopy.size = mIndexBufferSize;
				indicesCopy.srcOffset = mVertexBufferSize;
				cmd.copyBuffer(tempBuffer->GetBuffer(), mIndexBuffer->GetBuffer(), 1, &indicesCopy);
			}));
	}
} // Turbo
