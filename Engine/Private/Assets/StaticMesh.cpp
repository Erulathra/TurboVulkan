#include "Assets/StaticMesh.h"

#include "fastgltf/core.hpp"
#include "fastgltf/tools.hpp"
#include "fastgltf/glm_element_traits.hpp"

namespace Turbo
{
	constexpr std::string_view kPositionName = "POSITION";
	constexpr std::string_view kNormalName = "NORMAL";
	constexpr std::string_view kUVName = "TEXCOORD_0";
	constexpr std::string_view kColorName = "COLOR_0";

	void FMeshManager::Init(FGPUDevice& gpu)
	{

		FBufferBuilder bufferBuilder = {};
		bufferBuilder
			.Init(vk::BufferUsageFlagBits::eUniformBuffer, EBufferFlags::None, sizeof(FMeshPointers) * kMaxMeshes)
			.SetName(FName("MeshPointer"));

		mMeshPointersPool = gpu.CreateBuffer(bufferBuilder);
	}

	void FMeshManager::Destroy(FGPUDevice& gpu)
	{
		gpu.DestroyBuffer(mMeshPointersPool);
	}

	FDeviceAddress FMeshManager::GetMeshPointersAddress(FGPUDevice& gpu, THandle<FMesh> handle) const
	{
		const FBuffer* pointersPoolBuffer = gpu.AccessBuffer(mMeshPointersPool);
		TURBO_CHECK(pointersPoolBuffer);

		const FDeviceAddress memoryOffset = sizeof(FMeshPointers) * handle.GetIndex();
		TURBO_CHECK(memoryOffset < pointersPoolBuffer->GetSize());

		return pointersPoolBuffer->GetDeviceAddress() + memoryOffset;
	}

	void LogGLTFError(std::string_view message, fastgltf::Error error)
	{
		TURBO_LOG(LogMeshLoading, Error, "{} Error: {} Message: {}", message, fastgltf::getErrorName(error), fastgltf::getErrorMessage(error));
	}

	template<typename T>
	void LoadComponentBuffer( const fastgltf::Asset& meshAsset, THandle<FBuffer>& outBuffer, FDeviceAddress& outDeviceAddress, std::string_view attributeName)
	{
		const fastgltf::Mesh& gltfMesh = meshAsset.meshes.front();
		const fastgltf::Primitive& gltfSubmesh = gltfMesh.primitives.front();

		if (auto attribute = gltfSubmesh.findAttribute(attributeName);
			attribute != gltfSubmesh.attributes.cend())
		{
			const fastgltf::Accessor& accessor = meshAsset.accessors[attribute->accessorIndex];

			std::vector<T> componentData;
			componentData.reserve(accessor.count);

			fastgltf::iterateAccessor<T>(meshAsset, accessor, [&](const T& vertex)
			{
				componentData.push_back(vertex);
			});

			FBufferBuilder bufferBuilder = {};
			bufferBuilder.Init(
				vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst,
				EBufferFlags::None,
				componentData.size() * sizeof(T)
			);
			bufferBuilder.SetData(componentData.data());
			bufferBuilder.SetName(FName(fmt::format("{}_{}", gltfMesh.name, attributeName)));

			FGPUDevice& gpu = entt::locator<FGPUDevice>::value();
			outBuffer = gpu.CreateBuffer(bufferBuilder);
			FBuffer* outBufferData = gpu.AccessBuffer(outBuffer);
			outDeviceAddress = outBufferData->GetDeviceAddress();
		}
	}

	template<>
	bool FMeshLoader::TryLoadAsset(FName assetPath, THandle<FMesh> assetHandle, FMesh& outLoadedAsset)
	{
		// This method is as much naive as it can be, but for now it should last.
		TURBO_LOG(LogMeshLoading, Info, "Loading GLTF mesh. ({})", assetPath.ToCString())
		std::filesystem::path fileSystemPath = assetPath.ToString();

		fastgltf::Expected<fastgltf::GltfDataBuffer> data = fastgltf::GltfDataBuffer::FromPath(fileSystemPath);
		if (data.error() != fastgltf::Error::None)
		{
			LogGLTFError("Loading error.", data.error());
			return false;
		}

		fastgltf::Parser parser;
		fastgltf::Expected<fastgltf::Asset> meshAsset = parser.loadGltf(data.get(), fileSystemPath, fastgltf::Options::GenerateMeshIndices);
		if (meshAsset.error() != fastgltf::Error::None)
		{
			LogGLTFError("Parsing error.", meshAsset.error());
			return false;
		}

		TURBO_CHECK_MSG(meshAsset->meshes.size() == 1, "Only one mesh per file is supported");
		TURBO_CHECK_MSG(meshAsset->meshes.front().primitives.size() == 1, "Only one submesh is supported");

		FGPUDevice& gpu = entt::locator<FGPUDevice>::value();
		FMeshManager& meshManager = entt::locator<FMeshManager>::value();

		fastgltf::Mesh& gltfMesh = meshAsset->meshes.front();

		// load mesh
		constexpr uint32 subMeshId = 0;
		fastgltf::Primitive& glftSubMesh = gltfMesh.primitives[subMeshId];

		TURBO_CHECK(glftSubMesh.indicesAccessor)
		{
			const fastgltf::Accessor& indicesAccessor = meshAsset->accessors[glftSubMesh.indicesAccessor.value()];

			std::vector<uint32> indices;
			indices.reserve(indicesAccessor.count);
			outLoadedAsset.mVertexCount = indicesAccessor.count;
			outLoadedAsset.mName = assetPath;

			fastgltf::iterateAccessor<uint32>(meshAsset.get(), indicesAccessor, [&](uint32 index)
			{
				indices.push_back(index);
			});

			FBufferBuilder bufferBuilder = {};
			bufferBuilder.Init(
				vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst,
				EBufferFlags::None,
				indices.size() * sizeof(uint32)
			);
			bufferBuilder.SetData(indices.data());
			bufferBuilder.SetName(FName(fmt::format("{}_INDICES", meshAsset->meshes.front().name)));

			outLoadedAsset.mIndicesBuffer = gpu.CreateBuffer(bufferBuilder);
		}

		FMeshPointers meshPointers = {};

		LoadComponentBuffer<glm::vec3>(meshAsset.get(), outLoadedAsset.mPositionBuffer, meshPointers.mPositionBuffer, kPositionName);
		LoadComponentBuffer<glm::vec3>(meshAsset.get(), outLoadedAsset.mNormalBuffer, meshPointers.mNormalBuffer, kNormalName);
		LoadComponentBuffer<glm::vec2>(meshAsset.get(), outLoadedAsset.mUVBuffer, meshPointers.mUVBuffer, kUVName);
		LoadComponentBuffer<glm::vec4>(meshAsset.get(), outLoadedAsset.mColorBuffer, meshPointers.mColorBuffer, kColorName);

		gpu.ImmediateSubmit(FOnImmediateSubmit::CreateLambda([&](FCommandBuffer& cmd)
		{
			const FBufferBuilder stagingBufferBuilder = FBufferBuilder::CreateStagingBuffer(&meshPointers, sizeof(FMeshPointers));
			const THandle<FBuffer> stagingBuffer = gpu.CreateBuffer(stagingBufferBuilder);
			cmd.BufferBarrier(
				stagingBuffer,
				vk::AccessFlagBits2::eHostWrite,
				vk::PipelineStageFlagBits2::eHost,
				vk::AccessFlagBits2::eTransferRead,
				vk::PipelineStageFlagBits2::eTransfer
				);

			const FCopyBufferInfo copyBufferInfo = {
				.mSrc = stagingBuffer,
				.mDst = meshManager.mMeshPointersPool,
				.mDstOffset = sizeof(FMeshPointers) * assetHandle.GetIndex(),
				.mSize = sizeof(FMeshPointers)
			};

			cmd.CopyBuffer(copyBufferInfo);

			gpu.DestroyBuffer(stagingBuffer);
		}));

		return true;
	}

	template<>
	void FMeshLoader::UnloadAsset(THandle<FMesh> assetHandle, const FMesh& unloadedAsset)
	{
		const std::array buffersToDestroy = {
			unloadedAsset.mIndicesBuffer,
			unloadedAsset.mPositionBuffer,
			unloadedAsset.mNormalBuffer,
			unloadedAsset.mUVBuffer,
			unloadedAsset.mColorBuffer,
		};

		FGPUDevice& gpu = entt::locator<FGPUDevice>::value();
		for (const THandle<FBuffer>& handle : buffersToDestroy)
		{
			if (handle.IsValid())
			{
				gpu.DestroyBuffer(handle);
			}
		}
	}
} // Turbo