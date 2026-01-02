#include "World/World.h"

using namespace entt::literals;

namespace Turbo
{

	void FWorld::OpenLevel(FName path)
	{
	}

	void FWorld::UnloadLevel()
	{
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
