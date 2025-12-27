#pragma once

#include "GenericAssetManager.h"
#include "Graphics/GPUDevice.h"
#include "Graphics/GraphicsCore.h"

DECLARE_LOG_CATEGORY(LogMeshLoading, Display, Display)

namespace Turbo
{
	class FBuffer;
	struct FMaterial;

	// TODO: Replace with something more robust
	constexpr uint32 kMaxMeshes = 2048;

	struct FMesh final
	{
		THandle<FBuffer> mIndicesBuffer = {};
		THandle<FBuffer> mPositionBuffer = {};

		THandle<FBuffer> mNormalBuffer = {};
		THandle<FBuffer> mUVBuffer = {};
		THandle<FBuffer> mColorBuffer = {};

		uint32 mVertexCount = 0;

		FName mName;
	};

	struct FMeshPointers final
	{
		FDeviceAddress mPositionBuffer = kNullDeviceAddress;
		FDeviceAddress mNormalBuffer = kNullDeviceAddress;
		FDeviceAddress mUVBuffer = kNullDeviceAddress;
		FDeviceAddress mColorBuffer = kNullDeviceAddress;
	};

	using FMeshLoader = FAssetManager<FMesh>;

	struct FMeshManager
	{
		DELETE_COPY(FMeshManager)
		FMeshManager() = default;

	public:
		void Init(FGPUDevice& gpu);
		void Destroy(FGPUDevice& gpu);

		FDeviceAddress GetMeshPointersAddress(FGPUDevice& gpu, THandle<FMesh> handle) const;

	public:
		THandle<FBuffer> mMeshPointersPool;
	};


	template<>
	bool FMeshLoader::TryLoadAsset(FName assetPath, THandle<FMesh> assetHandle, FMesh& outLoadedAsset);
	template<>
	void FMeshLoader::UnloadAsset(THandle<FMesh> assetHandle, const FMesh& unloadedAsset);
} // Turbo