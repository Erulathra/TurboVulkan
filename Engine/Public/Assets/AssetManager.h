#pragma once

#include "StaticMesh.h"

namespace Turbo
{
	class FTexture;
	class FGPUDevice;

	// Replace with something more robust
	constexpr uint32 kMaxSubmeshes = 1024;

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
		std::vector<THandle<FMesh>> LoadMesh(const std::filesystem::path& path);

		FMesh* AccessMesh(THandle<FMesh> handle) { return mMeshPool.Access(handle); }
		const FMesh* AccessMesh(THandle<FMesh> handle) const { return mMeshPool.Access(handle); }

		FDeviceAddress GetMeshPointersAddress(FGPUDevice& gpu, THandle<FMesh> handle) const;

		void UnloadMesh(const std::vector<THandle<FMesh>>& meshesToUnload);
		/** Mesh interface end */

	public:
		THandle<FTexture> LoadTexture(const std::filesystem::path& path, bool bSRGB = true);

	private:
		THandle<FTexture> LoadDDS(const std::filesystem::path& path, bool bSRGB = true);

	private:
		TPoolGrowable<FMesh> mMeshPool;

		THandle<FBuffer> mSubMeshPointersPool;

	public:
		friend class FEngine;
	};
} // Turbo