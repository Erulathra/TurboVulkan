#pragma once

#include "StaticMesh.h"
#include "Core/DataStructures/GenPoolGrowable.h"

DECLARE_LOG_CATEGORY(LogAssetManager, Display, Display)
DECLARE_LOG_CATEGORY(LogMeshLoading, Display, Display)
DECLARE_LOG_CATEGORY(LogTextureLoading, Display, Display)

namespace Turbo
{
	class FTexture;
	class FGPUDevice;

	// Replace with something more robust
	constexpr uint32 kMaxMeshes = 1024;

	class FAssetManager final
	{
		DELETE_COPY(FAssetManager);

	public:
		FAssetManager() = default;

	public:
		void Init(FGPUDevice& gpu);
		void Destroy(FGPUDevice& gpu) const;

		/** Mesh interface */
	public:
		[[nodiscard]] THandle<FMesh> LoadMesh(FName path, bool bLevelAsset = true);
		void UnloadMesh(THandle<FMesh> meshHandle);

		[[nodiscard]] FMesh* AccessMesh(THandle<FMesh> handle) { return mMeshPool.Access(handle); }
		[[nodiscard]] const FMesh* AccessMesh(THandle<FMesh> handle) const { return mMeshPool.Access(handle); }

		[[nodiscard]] FDeviceAddress GetMeshPointersAddress(FGPUDevice& gpu, THandle<FMesh> handle) const;

		/** Mesh interface end */

		/** Texture interface */
	public:
		[[nodiscard]] THandle<FTexture> LoadTexture(FName path, bool bSRGB = true, bool bLevelAsset = true);
		void UnloadTexture(THandle<FTexture> handle);

	private:
		THandle<FTexture> LoadDDS(FName path, bool bSRGB = true);

		/** Texture interface end */

	private:
		template<typename AssetType>
		THandle<AssetType> FindCachedAsset(FName path)
		{
			if (auto foundIt = mAssetCache.find(path);
				foundIt != mAssetCache.end())
			{
				TURBO_LOG(LogAssetManager, Display, "Cache hit! ({})", path.ToString())
				return THandle<AssetType>(foundIt->second);
			}

			return {};
		}

	private:
		TGenPoolGrowable<FMesh> mMeshPool;
		THandle<FBuffer> mMeshPointersPool;

		entt::dense_map<FName, FHandle> mAssetCache;

	public:
		friend class FEngine;
	};
} // Turbo