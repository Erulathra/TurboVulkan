#include "World/GLTFSceneLoader.h"

#include "Assets/GLTFHelpers.h"
#include "Assets/MaterialManager.h"
#include "Core/FileSystem.h"
#include "World/MeshComponent.h"
#include "World/World.h"

#include "fastgltf/core.hpp"
#include "fastgltf/tools.hpp"

namespace
{
	void LogGLTFError(std::string_view message, fastgltf::Error error)
	{
		TURBO_LOG(LogGLTFSceneLoader, Error, "{} Error: {} Message: {}", message, fastgltf::getErrorName(error), fastgltf::getErrorMessage(error));
	}
}

namespace Turbo
{
	void FGLTFSceneLoader::LoadGLTFScene(FWorld& world, FName path)
	{
		TURBO_LOG(LogGLTFSceneLoader, Info, "Loading {} scene using gltf scene loader.", path)

		FTurboGLTFDataBuffer dataBuffer = FTurboGLTFDataBuffer::Load(path.ToString());

		fastgltf::Parser parser;
		fastgltf::Expected<fastgltf::Asset> gltfAsset = parser.loadGltf(
			dataBuffer,
			path.ToCString(),
			fastgltf::Options::GenerateMeshIndices | fastgltf::Options::DecomposeNodeMatrices
		);

		if (gltfAsset.error() != fastgltf::Error::None)
		{
			LogGLTFError("Parsing error.", gltfAsset.error());
			return;
		}

		const std::filesystem::path assetDirectory = std::filesystem::path(path.ToString()).parent_path();
		for (int bufferId = 0; bufferId < gltfAsset->buffers.size(); ++bufferId)
		{
			fastgltf::Buffer& buffer = gltfAsset->buffers[bufferId];
			if (std::holds_alternative<fastgltf::sources::URI>(buffer.data))
			{
				const fastgltf::sources::URI bufferURI = std::get<fastgltf::sources::URI>(buffer.data);
				const std::filesystem::path relativeBufferPath = assetDirectory / bufferURI.uri.path();

				std::vector<byte> bufferData;
				FileSystem::LoadData(relativeBufferPath.string(), bufferData);

				fastgltf::sources::Vector vectorSource = {
					std::move(bufferData)
				};

				buffer.data = std::move(vectorSource);
			}
		}

		FAssetManager& assetManager = entt::locator<FAssetManager>::value();

		/** TODO: Remove this with proper material */
		FMaterialManager& materialManager = entt::locator<FMaterialManager>::value();
		FPipelineBuilder pipelineBuilder = FMaterialManager::CreateOpaquePipeline("MeshTestMaterial.slang");
		THandle<FMaterial> materialHandle = materialManager.LoadMaterial<void, void>(pipelineBuilder, 1);
		THandle<FMaterial::Instance> materialInstanceHandle = materialManager.CreateMaterialInstance(materialHandle);

		std::vector<entt::entity> nodeEntities;
		nodeEntities.reserve(gltfAsset->nodes.size());

		// TODO: load meshes and textures ahead of time

		// Setup nodes
		for (const fastgltf::Node& node : gltfAsset->nodes)
		{
			entt::entity nodeEntity = world.mRegistry.create();
			nodeEntities.push_back(nodeEntity);

			world.mRegistry.emplace<FSpawnedByLevelTag>(nodeEntity);
			world.mRegistry.emplace<FRelationship>(nodeEntity);
			FTransform& transform = world.mRegistry.emplace<FTransform>(nodeEntity);

			TURBO_CHECK(std::holds_alternative<fastgltf::TRS>(node.transform))
			{
				const fastgltf::TRS& trs = std::get<fastgltf::TRS>(node.transform);

				transform.mPosition[0] = trs.scale[0];
				transform.mPosition[1] = trs.scale[1];
				transform.mPosition[2] = trs.scale[2];

				transform.mRotation[0] = trs.rotation[0];
				transform.mRotation[1] = trs.rotation[1];
				transform.mRotation[2] = trs.rotation[2];
				transform.mRotation[3] = trs.rotation[3];

				transform.mScale[0] = trs.scale[0];
				transform.mScale[1] = trs.scale[1];
				transform.mScale[2] = trs.scale[2];
			}

			if (node.meshIndex.has_value())
			{
				FMeshComponent& meshComponent = world.mRegistry.emplace<FMeshComponent>(nodeEntity);
				meshComponent.mMaterial = materialHandle;
				meshComponent.mMaterialInstance = materialInstanceHandle;

				FMeshLoadSettings meshLoadSettings = {
					.mMeshIndex = static_cast<uint32>(node.meshIndex.value())
				};

				meshComponent.mMesh = assetManager.LoadMeshGLTF(path, meshLoadSettings, gltfAsset.get());
			}
		}

		// Setup hierarchy
		for (size_t nodeId = 0; nodeId < gltfAsset->nodes.size(); ++nodeId)
		{
			const fastgltf::Node& gltfNode = gltfAsset->nodes[nodeId];
			for (std::size_t childId : gltfNode.children)
			{
				FSceneGraph::AddChild(world.mRegistry, nodeEntities[nodeId], nodeEntities[childId]);
			}
		}
	}
} // Turbo