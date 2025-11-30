#pragma once
#include "Graphics/GraphicsCore.h"

namespace Turbo
{
	class FBuffer;
	struct FMaterial;

	// TODO: Think about better memory layout
	static constexpr uint32 kMaxSubMeshesPerMesh = 15;

	struct FSubMesh final
	{
		THandle<FBuffer> mIndicesBuffer = {};
		THandle<FBuffer> mPositionBuffer = {};

		THandle<FBuffer> mNormalBuffer = {};
		THandle<FBuffer> mUVBuffer = {};
		THandle<FBuffer> mColorBuffer = {};

		uint32 mVertexCount = 0;
	};

	struct FMeshPointers final
	{
		DeviceAddress mPositionBuffer = NullDeviceAddress;
		DeviceAddress mNormalBuffer = NullDeviceAddress;
		DeviceAddress mUVBuffer = NullDeviceAddress;
		DeviceAddress mColorBuffer = NullDeviceAddress;
	};

	struct FMesh final
	{
		std::array<THandle<FSubMesh>, kMaxSubMeshesPerMesh> mSubMeshes;
		uint32 mNumSubMeshes = 0;
	};

} // Turbo