#pragma once

#include "StaticMesh.h"

namespace Turbo
{
	class FAssetManager final
	{
	private:
		FAssetManager();

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