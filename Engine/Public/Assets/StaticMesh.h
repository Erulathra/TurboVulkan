#pragma once
#include "Graphics/GraphicsCore.h"

namespace Turbo
{
	class FBuffer;
	struct FMaterial;

	// TODO: Think about better memory layout
	static constexpr uint32 kMaxSubMeshesPerMesh = 15;

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

} // Turbo