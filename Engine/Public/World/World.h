#pragma once

#include "Assets/AssetManager.h"
#include "World/SceneGraph.h"
#include "Core/DataStructures/Handle.h"

namespace Turbo
{
	class FCamera;
	struct FMesh;

	struct FSpawnedByLevelTag {};

	struct FRuntimeLevel
	{
		std::vector<THandle<FMesh>> mLoadedMeshes;
		std::vector<THandle<FTexture>> mLoadedTextures;
	};

	class FWorld
	{
	public:
		void OpenLevel(FName path);
		void UnloadLevel();

	public:
		entt::registry mRegistry;
		FRuntimeLevel mRuntimeLevel;
	};
} // Turbo
