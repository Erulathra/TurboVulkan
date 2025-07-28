#include "GLTFMeshLoader.h"

#include <fastgltf/tools.hpp>
#include <fastgltf/glm_element_traits.hpp>

#include "Core/RHI/RHIMesh.h"

constexpr std::string kPositionName = "POSITION";
constexpr std::string kNormalName = "NORMAL";
constexpr std::string kUVName = "TEXCOORD_0";
constexpr std::string kColorName = "COLOR_0";

Turbo::FGLTFMeshLoader::~FGLTFMeshLoader() = default;

std::shared_ptr<Turbo::FRHIMesh> Turbo::FGLTFMeshLoader::LoadMesh(FVulkanDevice* device, const std::filesystem::path& path, ELoadMeshFlags loadMeshFlags)
{
	TURBO_LOG(LOG_STREAMING, Info, "Loading GLTF mesh: {}.", path.c_str());

	// Currently only this type is supported
	TURBO_ENSURE(loadMeshFlags == ELoadMeshFlags::LoadVertexWithColor);

	fastgltf::Expected<fastgltf::GltfDataBuffer> meshData = fastgltf::GltfDataBuffer::FromPath(path);
	if (meshData.error() != fastgltf::Error::None)
	{
		TURBO_LOG(LOG_STREAMING, Error, "GLTF Mesh load error. Reason: {}", magic_enum::enum_name(meshData.error()));
		return nullptr;
	}

	fastgltf::Expected<fastgltf::Asset> meshAsset = mParser.loadGltf(meshData.get(), path.parent_path());
	if (meshAsset.error() != fastgltf::Error::None)
	{
		TURBO_LOG(LOG_STREAMING, Error, "GLTF Mesh parsing error. Reason: {}", magic_enum::enum_name(meshAsset.error()));
		return nullptr;
	}

	TURBO_CHECK_MSG(meshAsset->meshes.size() == 1, "Only one mesh per file is supported");

	FRHIMeshCreationInfo meshCreateInfo;

	size_t numAllVertices = 0;
	size_t numAllIndices = 0;
	for (const fastgltf::Primitive& subMesh : meshAsset->meshes.front().primitives)
	{
		fastgltf::Accessor& positionAccessor = meshAsset->accessors[subMesh.findAttribute(kPositionName)->accessorIndex];
		fastgltf::Accessor& indicesAccessor = meshAsset->accessors[subMesh.indicesAccessor.value()];

		numAllVertices += positionAccessor.count;
		numAllIndices += indicesAccessor.count;
	}

	meshCreateInfo.Reserve(numAllVertices, numAllIndices);

	for (const fastgltf::Primitive& subMesh : meshAsset->meshes.front().primitives)
	{
		const uint32 numIndices = meshAsset->accessors[subMesh.indicesAccessor.value()].count;
		meshCreateInfo.SubMeshes.emplace_back(meshCreateInfo.Indices.size(), numIndices);

		// Load indices
		{
			fastgltf::Accessor& indicesAccessor = meshAsset->accessors[subMesh.indicesAccessor.value()];
			fastgltf::iterateAccessor<uint32>(meshAsset.get(), indicesAccessor, [&](uint32 index)
			{
				meshCreateInfo.Indices.push_back(index);
			});
		}

		auto loadVertexComponent = [&]<typename T>(std::vector<T>& dst, const std::string_view componentName)
		{
			if (const fastgltf::Attribute* attribute = subMesh.findAttribute(componentName))
			{
				fastgltf::Accessor& accessor = meshAsset->accessors[attribute->accessorIndex];
				fastgltf::iterateAccessor<T>(meshAsset.get(), accessor, [&](T vertexComponent)
				{
					dst.push_back(vertexComponent);
				});
			}
		};

		loadVertexComponent(meshCreateInfo.Positions, kPositionName);
		loadVertexComponent(meshCreateInfo.Normals, kNormalName);
		loadVertexComponent(meshCreateInfo.UVs, kUVName);
		loadVertexComponent(meshCreateInfo.Colors, kColorName);
	}

	return FRHIMesh::CreateShared(device, meshCreateInfo);
}
