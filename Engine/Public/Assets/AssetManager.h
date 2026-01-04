#pragma once

#include "Assets/StaticMesh.h"
#include "Assets/AssetManagerHelpers.h"
#include "Core/DataStructures/GenPoolGrowable.h"
#include "Core/DataStructures/ManualPoolGrowable.h"

DECLARE_LOG_CATEGORY(LogAssetManager, Display, Display)
DECLARE_LOG_CATEGORY(LogMeshLoading, Display, Display)
DECLARE_LOG_CATEGORY(LogTextureLoading, Display, Display)

namespace fastgltf
{
	class Asset;
}

namespace Turbo
{
	class FBuffer;
	class FTexture;
	struct FMesh;
	class FGPUDevice;


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
		[[nodiscard]] THandle<FMesh> LoadMesh(FName assetPath, const FMeshLoadSettings& meshLoadSettings = FMeshLoadSettings());
		[[nodiscard]] THandle<FMesh> LoadMeshGLTF(FName assetPath, const FMeshLoadSettings& meshLoadSettings, fastgltf::Asset& loadedAsset);

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
		THandle<AssetType> FindCachedAsset(uint32 hash)
		{
			if (auto foundIt = mAssetCache.find(hash);
				foundIt != mAssetCache.end())
			{
				TURBO_LOG(LogAssetManager, Display, "Asset manager cache hit!")
				return THandle<AssetType>(foundIt->second);
			}

			return {};
		}

	private:
		TGenPoolGrowable<FMesh> mMeshPool;
		THandle<FBuffer> mMeshPointersPool;

		TManualPoolGrowable<FTextureAsset> mTexturePool;

		entt::dense_map<uint32, FHandle> mAssetCache;

	public:
		friend class FEngine;
	};
} // Turbo