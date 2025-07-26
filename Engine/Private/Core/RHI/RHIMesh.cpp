#include "Core/RHI/RHIMesh.h"

#include "Core/RHI/VulkanDevice.h"

namespace Turbo {
	bool FRHIMeshCreationInfo::IsValid() const
	{
		const size_t numVertices = Positions.size();
		return (Normals.empty() || Normals.size() == numVertices)
			&& (UVs.empty() || UVs.size() == numVertices)
			&& (Colors.empty() || Colors.size() == numVertices);
	}

	size_t FRHIMeshCreationInfo::GetVertexBufferSize() const
	{
		return Positions.size() * sizeof(glm::vec3)
			+ Normals.size() * sizeof(glm::vec3)
			+ UVs.size() * sizeof(glm::vec2)
			+ Colors.size() * sizeof(glm::vec4);
	}

	std::shared_ptr<FRHIMesh> FRHIMesh::CreateShared(FVulkanDevice* device, const FRHIMeshCreationInfo& creationInfo)
	{
		std::shared_ptr<FRHIMesh> result(new FRHIMesh());
		result->Init(device, creationInfo);

		return result;
	}

	std::unique_ptr<FRHIMesh> FRHIMesh::CreateUnique(FVulkanDevice* device, const FRHIMeshCreationInfo& creationInfo)
	{
		std::unique_ptr<FRHIMesh> result(new FRHIMesh());
		result->Init(device, creationInfo);

		return result;
	}

	void FRHIMesh::Init(FVulkanDevice* device, const FRHIMeshCreationInfo& creationInfo)
	{
		TRACE_ZONE_SCOPED();
		TURBO_CHECK(device);

		TURBO_CHECK(creationInfo.IsValid());

		if (creationInfo.SubMeshes.empty())
		{
			mSubMeshes.emplace_back(0, creationInfo.Indices.size());
		}

		const size_t vertexBufferSize = creationInfo.GetVertexBufferSize();
		const size_t indexBufferSize = creationInfo.Indices.size() * sizeof(uint32);

		mVertexBuffer = FBuffer::CreateShared(
			device,
			vertexBufferSize,
			vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eShaderDeviceAddress,
			vma::MemoryUsage::eAutoPreferDevice
			);

		mIndexBuffer = FBuffer::CreateShared(
			device,
			indexBufferSize,
			vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst,
			vma::MemoryUsage::eAutoPreferDevice
			);

		mPositionsAddress = device->GetBufferAddress(mVertexBuffer.get());

		std::unique_ptr<FBuffer> tempBuffer = FBuffer::CreateUnique(
			device,
			vertexBufferSize + indexBufferSize,
			vk::BufferUsageFlagBits::eTransferSrc,
			vma::MemoryUsage::eAutoPreferHost
			);

		// Copy from vectors to buffer
		byte* tempBufferData = tempBuffer->GetMappedData();
		size_t bufferOffset = 0;

		auto copyBufferToGPU = [&]<typename T>(const std::vector<T>& buffer, EVertexComponents component, vk::DeviceAddress& outAddress)
		{
			if (!buffer.empty())
			{
				size_t bufferSize = buffer.size() * sizeof(T);
				std::memcpy(tempBufferData + bufferOffset, buffer.data(), bufferSize);

				mVertexComponents |= component;
				outAddress = mPositionsAddress + bufferOffset;
				bufferOffset += bufferSize;
			}
		};

		copyBufferToGPU(creationInfo.Positions, EVertexComponents::Position, mPositionsAddress);
		copyBufferToGPU(creationInfo.Normals, EVertexComponents::Normal, mNormalsAddress);
		copyBufferToGPU(creationInfo.UVs, EVertexComponents::UV, mUvAddress);
		copyBufferToGPU(creationInfo.Colors, EVertexComponents::Color, mColorAddress);

		std::memcpy(tempBufferData + vertexBufferSize, creationInfo.Indices.data(), indexBufferSize);

		// Copy from host to device
		gEngine->GetRHI()->SubmitImmediateCommand(
			FSubmitDelegate::CreateLambda([&](vk::CommandBuffer cmd)
			{
				vk::BufferCopy verticesCopy{};
				verticesCopy.size = vertexBufferSize;
				cmd.copyBuffer(tempBuffer->GetBuffer(), mVertexBuffer->GetBuffer(), 1, &verticesCopy);

				vk::BufferCopy indicesCopy{};
				indicesCopy.size = indexBufferSize;
				indicesCopy.srcOffset = vertexBufferSize;
				cmd.copyBuffer(tempBuffer->GetBuffer(), mIndexBuffer->GetBuffer(), 1, &indicesCopy);
			}));
	}

	void FRHIMesh::Draw(const vk::CommandBuffer cmd, uint32 instanceCount)
	{
		cmd.bindIndexBuffer(mIndexBuffer->GetBuffer(), 0, vk::IndexType::eUint32);

		for (const FSubMesh& subMesh : mSubMeshes)
		{
			cmd.drawIndexed(subMesh.NumIndices, instanceCount, subMesh.StartVertexIndex, 0, 0);
		}
	}
} // Turbo