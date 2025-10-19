#include "Assets/AssetManager.h"

#include "Assets/StaticMesh.h"
#include "Core/Engine.h"
#include "Graphics/GPUDevice.h"

#include <fastgltf/core.hpp>
#include <fastgltf/tools.hpp>
#include <fastgltf/glm_element_traits.hpp>

namespace Turbo
{
	constexpr std::string_view kPositionName = "POSITION";
	constexpr std::string_view kNormalName = "NORMAL";
	constexpr std::string_view kUVName = "TEXCOORD_0";
	constexpr std::string_view kColorName = "COLOR_0";

	FAssetManager::FAssetManager() = default;

	void LogGLTFError(std::string_view message, fastgltf::Error error)
	{
		TURBO_LOG(LOG_MESH_LOADING, Error, "{} Error: {} Message: {}", message, fastgltf::getErrorName(error), fastgltf::getErrorMessage(error));
	}

	template<typename T>
	void LoadComponentBuffer( const fastgltf::Asset& meshAsset, THandle<FBuffer>& buffer, std::string_view attributeName)
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

			buffer = gEngine->GetGpu()->CreateBuffer(bufferBuilder);
		}
	}

	THandle<FSubMesh> FAssetManager::LoadMesh(const std::filesystem::path& path)
	{
		// This method is as much naive as it can be, but for now it should last.
		TURBO_LOG(LOG_MESH_LOADING, Info, "Loading GLTF mesh. ({})", path.string())
		THandle<FSubMesh> Result = {};

		fastgltf::Expected<fastgltf::GltfDataBuffer> data = fastgltf::GltfDataBuffer::FromPath(path);
		if (data.error() != fastgltf::Error::None)
		{
			LogGLTFError("Loading error.", data.error());
			return Result;
		}

		fastgltf::Parser parser(fastgltf::Extensions::KHR_draco_mesh_compression);
		fastgltf::Expected<fastgltf::Asset> meshAsset = parser.loadGltf(data.get(), path, fastgltf::Options::GenerateMeshIndices);
		if (meshAsset.error() != fastgltf::Error::None)
		{
			LogGLTFError("Parsing error.", meshAsset.error());
			return Result;
		}

		TURBO_CHECK_MSG(meshAsset->meshes.size() == 1, "Only one mesh per file is supported");
		TURBO_CHECK_MSG(meshAsset->meshes.front().primitives.size() == 1, "Only one submesh is supported");

		const fastgltf::Primitive& sourceSubMesh = meshAsset->meshes.front().primitives.front();

		Result = mSubMeshPool.Acquire();
		FSubMesh* loadedSubMesh = mSubMeshPool.Access(Result);

		// Load Indices
		TURBO_CHECK(sourceSubMesh.indicesAccessor)
		{
			const fastgltf::Accessor& indicesAccessor = meshAsset->accessors[sourceSubMesh.indicesAccessor.value()];

			std::vector<uint32> indices;
			indices.reserve(indicesAccessor.count);
			loadedSubMesh->mVertexCount = indicesAccessor.count;

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

			loadedSubMesh->mIndicesBuffer = gEngine->GetGpu()->CreateBuffer(bufferBuilder);
		}

		LoadComponentBuffer<glm::vec3>(meshAsset.get(), loadedSubMesh->mPositionBuffer, kPositionName);
		LoadComponentBuffer<glm::vec3>(meshAsset.get(), loadedSubMesh->mNormalBuffer, kNormalName);
		LoadComponentBuffer<glm::vec2>(meshAsset.get(), loadedSubMesh->mUVBuffer, kUVName);
		LoadComponentBuffer<glm::vec4>(meshAsset.get(), loadedSubMesh->mColorBuffer, kColorName);

		return Result;
	}

	void FAssetManager::UnloadMesh(THandle<FSubMesh> meshToUnload)
	{
		const FSubMesh* staticMesh = mSubMeshPool.Access(meshToUnload);
		FGPUDevice* gpu = gEngine.get()->GetGpu();

		const std::array buffersToDestroy = {
			staticMesh->mIndicesBuffer,
			staticMesh->mPositionBuffer,
			staticMesh->mNormalBuffer,
			staticMesh->mUVBuffer,
			staticMesh->mColorBuffer,
		};

		for (const THandle<FBuffer>& handle : buffersToDestroy)
		{
			if (handle.IsValid())
			{
				gpu->DestroyBuffer(handle);
			}
		}
	}
} // Turbo