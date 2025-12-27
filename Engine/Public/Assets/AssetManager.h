#pragma once

#include "StaticMesh.h"
#include "Core/DataStructures/GenPoolGrowable.h"

DECLARE_LOG_CATEGORY(LogTextureLoading, Display, Display)

namespace Turbo
{
	class FTexture;
	class FGPUDevice;

	class FOldAssetManager final
	{
		DELETE_COPY(FOldAssetManager);

	public:
		FOldAssetManager() = default;

	public:
		void Init(FGPUDevice& gpu);
		void Destroy(FGPUDevice& gpu) const;

	public:
		THandle<FTexture> LoadTexture(const std::filesystem::path& path, bool bSRGB = true);

	private:
		THandle<FTexture> LoadDDS(const std::filesystem::path& path, bool bSRGB = true);

	private:
		TGenPoolGrowable<FMesh> mMeshPool;

		THandle<FBuffer> mMeshPointersPool;

	public:
		friend class FEngine;
	};
} // Turbo