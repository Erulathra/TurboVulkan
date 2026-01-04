#include "World/World.h"

#include "World/GLTFSceneLoader.h"

using namespace entt::literals;

namespace Turbo
{
	void FWorld::OpenLevel(FName path)
	{
		std::filesystem::path scenePath(path.ToString());
		if (scenePath.extension() == ".glb" || scenePath.extension() == ".gltf")
		{
			FGLTFSceneLoader::LoadGLTFScene(*this, path);
		}
	}

	void FWorld::UnloadLevel()
	{
		const auto spawnedByLevelView = mRegistry.view<FSpawnedByLevelTag>();
		mRegistry.destroy(spawnedByLevelView.begin(), spawnedByLevelView.end());

		FAssetManager& assetManager = entt::locator<FAssetManager>::value();
		for (THandle<FMesh> mesh : mRuntimeLevel.mLoadedMeshes)
		{
			assetManager.UnloadMesh(mesh);
		}

		for (THandle<FTexture> texture : mRuntimeLevel.mLoadedTextures)
		{
			assetManager.UnloadTexture(texture);
		}
	}


} // Turbo
