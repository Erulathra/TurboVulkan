#include "World/World.h"

#include "ProfilingMacros.h"
#include "World/GLTFSceneLoader.h"
#include "Assets/AssetManager.h"

using namespace entt::literals;

namespace Turbo
{
	void FWorld::OpenLevel(FName path)
	{
      TRACE_ZONE_SCOPED_FORMAT(OpenLevel, "Open Level ({})", path.ToString())

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
