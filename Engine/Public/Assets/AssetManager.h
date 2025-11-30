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
		THandle<FMesh> LoadMesh(const std::filesystem::path& path);
		FMesh* AccessMesh(THandle<FMesh> handle) { return mMeshPool.Access(handle); }
		FSubMesh* AccessSubMesh(THandle<FSubMesh> handle) { return mSubMeshPool.Access(handle); }
		DeviceAddress GetMeshPointersAddress(FGPUDevice& GpuDevice, THandle<FSubMesh> handle) const;

		void UnloadMesh(THandle<FMesh> meshToUnload);
		/** Mesh interface end */

	public:
		THandle<FTexture> LoadTexture(const std::filesystem::path& path, bool bSRGB = true);

	private:
		THandle<FTexture> LoadDDS(const std::filesystem::path& path, bool bSRGB = true);

	private:
		TPoolGrowable<FMesh> mMeshPool;
		TPoolGrowable<FSubMesh> mSubMeshPool;

		THandle<FBuffer> mSubMeshPointersPool;

	public:
		friend class FEngine;
	};
} // Turbo