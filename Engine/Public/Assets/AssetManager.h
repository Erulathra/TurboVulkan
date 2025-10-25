#pragma once

#include "StaticMesh.h"

namespace Turbo
{
	class FAssetManager final
	{
		DELETE_COPY(FAssetManager);

	public:
		FAssetManager() = default;

	public:
		THandle<FSubMesh> LoadMesh(const std::filesystem::path& path);
		FSubMesh* AccessMesh(THandle<FSubMesh> handle) { return mSubMeshPool.Access(handle); }
		void UnloadMesh(THandle<FSubMesh> meshToUnload);

	private:
		TPoolGrowable<FSubMesh> mSubMeshPool;

	public:
		friend class FEngine;
	};
} // Turbo