#include "World/GLTFSceneLoader.h"

#include "Assets/EngineResources.h"
#include "Assets/GLTFHelpers.h"
#include "Assets/MaterialManager.h"
#include "World/MeshComponent.h"
#include "World/World.h"

#include "fastgltf/core.hpp"
#include "fastgltf/tools.hpp"
#include "Graphics/GPUDevice.h"

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
		std::array<THandle<FMaterial::Instance>, kMaxSubMeshesPerMesh> mMaterials;
		uint32 mNumSubmeshes = 0;
	};

	void FGLTFSceneLoader::LoadGLTFScene(FWorld& world, FName path)
	{
		TURBO_LOG(LogGLTFSceneLoader, Info, "Loading {} scene using gltf scene loader.", path)
		const std::filesystem::path baseAssetPath = std::filesystem::path(path.ToString()).parent_path();

		FTurboGLTFDataBuffer dataBuffer = FTurboGLTFDataBuffer::Load(path.ToString());

		// Load gltf material
		fastgltf::Parser parser;
		fastgltf::Expected<fastgltf::Asset> gltfAsset = parser.loadGltf(
			dataBuffer,
			path.ToString(),
			fastgltf::Options::GenerateMeshIndices | fastgltf::Options::DecomposeNodeMatrices
		);

		if (gltfAsset.error() != fastgltf::Error::None)
		{
			LogGLTFError("Parsing error.", gltfAsset.error());
			return;
		}

		GLTF::LoadExternalBuffers(gltfAsset.get(), path.ToString());

		FAssetManager& assetManager = entt::locator<FAssetManager>::value();

		// Load Textures
		std::vector<THandle<FTexture>> loadedTextures;
		loadedTextures.reserve(gltfAsset->textures.size());
		for (uint32 textureId = 0; textureId < gltfAsset->textures.size(); ++textureId)
		{
			const fastgltf::Texture& gltfTexture = gltfAsset->textures[textureId];
			if (gltfTexture.imageIndex.has_value())
			{
				const fastgltf::Image& gltfImage = gltfAsset->images[gltfTexture.imageIndex.value()];
				if (std::holds_alternative<fastgltf::sources::URI>(gltfImage.data))
				{
					const fastgltf::sources::URI& uri = std::get<fastgltf::sources::URI>(gltfImage.data);

					const std::filesystem::path texturePath = baseAssetPath / uri.uri.path();
					THandle<FTexture> textureHandle = assetManager.LoadTexture(FName(texturePath.string()));
					loadedTextures.push_back(textureHandle);
				}
			}
		}

		FGPUDevice& gpu = entt::locator<FGPUDevice>::value();
		FMaterialManager& materialManager = entt::locator<FMaterialManager>::value();
		THandle<FMaterial> opaqueMaterial = materialManager.GetMaterial(EngineMaterials::kOpaqueBasePass);
		std::vector<THandle<FMaterial::Instance>> materialInstanceHandles;
		materialInstanceHandles.reserve(gltfAsset->materials.size());

		// Load and submit Materials
		gpu.ImmediateSubmit(
			FOnImmediateSubmit::CreateLambda(
				[&](FCommandBuffer& cmd)
				{
					for (const fastgltf::Material& gltfMaterial : gltfAsset->materials)
					{
						if (gltfMaterial.alphaMode != fastgltf::AlphaMode::Opaque)
						{
							// We don't support transparent materials for now
							TURBO_LOG(LogGLTFSceneLoader, Warn, "Unimplemented material. Skipping...")
							materialInstanceHandles.emplace_back();
							continue;
						}

						const fastgltf::PBRData& pbrData = gltfMaterial.pbrData;

						THandle<FMaterial::Instance> materialInstanceHandle = materialManager.CreateMaterialInstance(opaqueMaterial);
						EngineMaterials::FBasePassInstanceData instanceData = {};

						if (pbrData.baseColorTexture.has_value())
						{
							instanceData.mBaseColorTexture = loadedTextures[pbrData.baseColorTexture.value().textureIndex].GetIndex();
						}
						else
						{
							instanceData.mBaseColorTexture = EngineResources::GetWhiteTexture().GetIndex();
						}

						if (pbrData.metallicRoughnessTexture.has_value())
						{
							instanceData.mMetalicRoughnessTexture = loadedTextures[pbrData.metallicRoughnessTexture.value().textureIndex].GetIndex();
						}

						if (gltfMaterial.normalTexture.has_value())
						{
							instanceData.mNormalTexture = loadedTextures[gltfMaterial.normalTexture.value().textureIndex].GetIndex();
							instanceData.mNormalScale = gltfMaterial.normalTexture.value().scale;
						}

						instanceData.mBaseColorFactor = glm::float4{
							pbrData.baseColorFactor[0],
							pbrData.baseColorFactor[1],
							pbrData.baseColorFactor[2],
							pbrData.baseColorFactor[3],
						};

						instanceData.mMetalicFactor = pbrData.metallicFactor;
						instanceData.mRoughnessFactor = pbrData.roughnessFactor;

						materialManager.UpdateMaterialInstance<EngineMaterials::FBasePassInstanceData>(cmd, materialInstanceHandle, &instanceData);
						materialInstanceHandles.push_back(materialInstanceHandle);
					}
				})
		);

		// Load all meshes and submeshes
		std::vector<FSceneMeshNodeData> meshes;
		meshes.reserve(gltfAsset->meshes.size());
		for (uint32 meshId = 0; meshId < gltfAsset->meshes.size(); ++meshId)
		{
			const fastgltf::Mesh& gltfMesh = gltfAsset->meshes[meshId];
			FSceneMeshNodeData& meshData = meshes.emplace_back();

			meshData.mNumSubmeshes = gltfMesh.primitives.size();
			TURBO_CHECK(meshData.mNumSubmeshes < kMaxSubMeshesPerMesh)

			for (uint32 subMeshId = 0; subMeshId < meshData.mNumSubmeshes; ++subMeshId)
			{
				FMeshLoadSettings meshLoadSettings = {
					.mMeshIndex = meshId,
					.mSubMeshIndex = subMeshId
				};
				meshData.mSubMeshes[subMeshId] = assetManager.LoadMeshGLTF(path, meshLoadSettings, gltfAsset.get());

				const fastgltf::Primitive& gltfPrimitive = gltfMesh.primitives[subMeshId];
				meshData.mMaterials[subMeshId] = materialInstanceHandles[gltfPrimitive.materialIndex.value()];
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
					// todo: remove this when translucent and masked materials will be ready
					if (meshNodeData.mMaterials[0].IsValid() == false)
					{
						continue;
					}

					FMeshComponent& meshComponent = world.mRegistry.emplace<FMeshComponent>(nodeEntity);
					meshComponent.mMaterial = opaqueMaterial;
					meshComponent.mMaterialInstance = meshNodeData.mMaterials[0];
					meshComponent.mMesh = meshNodeData.mSubMeshes[0];
				}
				else
				{
					for (uint32 subMeshId = 0; subMeshId < meshNodeData.mNumSubmeshes; ++subMeshId)
					{
						// todo: remove this when translucent and masked materials will be ready
						if (meshNodeData.mMaterials[subMeshId].IsValid() == false)
						{
							continue;
						}

						const entt::entity meshEntity = world.mRegistry.create();
						world.mRegistry.emplace<FSpawnedByLevelTag>(meshEntity);
						world.mRegistry.emplace<FRelationship>(meshEntity);
						FSceneGraph::AddChild(world.mRegistry, nodeEntity, meshEntity);

						FMeshComponent& meshComponent = world.mRegistry.emplace<FMeshComponent>(meshEntity);
						meshComponent.mMaterial = opaqueMaterial;
						meshComponent.mMaterialInstance = meshNodeData.mMaterials[subMeshId];
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