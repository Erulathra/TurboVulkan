#pragma once

#include "StaticMesh.h"

namespace Turbo
{
	class FTexture;

	class FAssetManager final
	{
		DELETE_COPY(FAssetManager);

	public:
		FAssetManager() = default;

	public:
		THandle<FSubMesh> LoadMesh(const std::filesystem::path& path);
		FSubMesh* AccessMesh(THandle<FSubMesh> handle) { return mSubMeshPool.Access(handle); }
		void UnloadMesh(THandle<FSubMesh> meshToUnload);

		THandle<FTexture> LoadTexture(const std::filesystem::path& path, bool bSRGB = true);

	private:
		THandle<FTexture> LoadDDS(const std::filesystem::path& path, bool bSRGB = true);

	private:
		TPoolGrowable<FSubMesh> mSubMeshPool;

	public:
		friend class FEngine;
	};
} // Turbo