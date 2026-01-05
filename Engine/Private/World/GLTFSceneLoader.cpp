#include "World/GLTFSceneLoader.h"

#include "Assets/GLTFHelpers.h"
#include "Assets/MaterialManager.h"
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
	constexpr uint32 kMaxSubMeshesPerMesh = 32;

	struct FSceneMeshNodeData
	{
		std::array<THandle<FMesh>, kMaxSubMeshesPerMesh> mSubMeshes;
		uint32 mNumSubmeshes = 0;
	};

	void FGLTFSceneLoader::LoadGLTFScene(FWorld& world, FName path)
	{
		TURBO_LOG(LogGLTFSceneLoader, Info, "Loading {} scene using gltf scene loader.", path)

		FTurboGLTFDataBuffer dataBuffer = FTurboGLTFDataBuffer::Load(path.ToString());

		// Load gltf material
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

		GLTF::LoadExternalBuffers(gltfAsset.get(), path.ToString());

		FAssetManager& assetManager = entt::locator<FAssetManager>::value();

		/** TODO: Remove this with proper material */
		FMaterialManager& materialManager = entt::locator<FMaterialManager>::value();
		FPipelineBuilder pipelineBuilder = FMaterialManager::CreateOpaquePipeline("MeshTestMaterial.slang");
		THandle<FMaterial> materialHandle = materialManager.LoadMaterial<void, void>(pipelineBuilder, 1);
		THandle<FMaterial::Instance> materialInstanceHandle = materialManager.CreateMaterialInstance(materialHandle);

		// Load all meshes and submeshes
		std::vector<FSceneMeshNodeData> meshes;
		meshes.resize(gltfAsset->meshes.size());
		for (uint32 meshId = 0; meshId < gltfAsset->meshes.size(); ++meshId)
		{
			const fastgltf::Mesh& gltfMesh = gltfAsset->meshes[meshId];
			FSceneMeshNodeData& meshData = meshes[meshId];

			meshData.mNumSubmeshes = gltfMesh.primitives.size();
			TURBO_CHECK(meshData.mNumSubmeshes < kMaxSubMeshesPerMesh)

			for (uint32 subMeshId = 0; subMeshId < meshData.mNumSubmeshes; ++subMeshId)
			{
				FMeshLoadSettings meshLoadSettings = {
					.mMeshIndex = meshId,
					.mSubMeshIndex = subMeshId
				};
				meshData.mSubMeshes[subMeshId] = assetManager.LoadMeshGLTF(path, meshLoadSettings, gltfAsset.get());
			}
		}

		// Create node entities
		std::vector<entt::entity> nodeEntities;
		nodeEntities.reserve(gltfAsset->nodes.size());

		for (const fastgltf::Node& node : gltfAsset->nodes)
		{
			entt::entity nodeEntity = world.mRegistry.create();
			world.mRegistry.emplace<FSpawnedByLevelTag>(nodeEntity);
			world.mRegistry.emplace<FRelationship>(nodeEntity);

			nodeEntities.push_back(nodeEntity);

			FTransform& transform = world.mRegistry.emplace<FTransform>(nodeEntity);
			TURBO_CHECK(std::holds_alternative<fastgltf::TRS>(node.transform))
			{
				const fastgltf::TRS& trs = std::get<fastgltf::TRS>(node.transform);

				transform.mPosition[0] = trs.translation[0];
				transform.mPosition[1] = trs.translation[1];
				transform.mPosition[2] = trs.translation[2];

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
				const FSceneMeshNodeData& meshNodeData = meshes[node.meshIndex.value()];

				if (meshNodeData.mNumSubmeshes == 1)
				{
					FMeshComponent& meshComponent = world.mRegistry.emplace<FMeshComponent>(nodeEntity);
					meshComponent.mMaterial = materialHandle;
					meshComponent.mMaterialInstance = materialInstanceHandle;

					meshComponent.mMesh = meshNodeData.mSubMeshes[0];
				}
				else
				{
					for (uint32 subMeshId = 0; subMeshId < meshNodeData.mNumSubmeshes; ++subMeshId)
					{
						const entt::entity meshEntity = world.mRegistry.create();
						world.mRegistry.emplace<FSpawnedByLevelTag>(meshEntity);
						world.mRegistry.emplace<FRelationship>(meshEntity);
						FSceneGraph::AddChild(world.mRegistry, nodeEntity, meshEntity);

						FMeshComponent& meshComponent = world.mRegistry.emplace<FMeshComponent>(meshEntity);
						meshComponent.mMaterial = materialHandle;
						meshComponent.mMaterialInstance = materialInstanceHandle;
						meshComponent.mMesh = meshNodeData.mSubMeshes[subMeshId];
					}
				}
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