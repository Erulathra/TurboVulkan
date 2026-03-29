#pragma once

#include "Assets/AssetManagerHelpers.h"
#include "Graphics/GraphicsCore.h"

namespace Turbo
{
	struct FBuffer;
	struct FMaterial;

	struct FBounds
	{
		glm::float3 mMin = glm::float3(std::numeric_limits<float>::lowest());
		float mRadius = std::numeric_limits<float>::lowest();
		glm::float3 mMax = glm::float3(std::numeric_limits<float>::max());
		float mRadiusSquared = std::numeric_limits<float>::lowest();
	};

	struct FMesh final
	{
		THandle<FBuffer> mIndicesBuffer = {};
		THandle<FBuffer> mPositionBuffer = {};

		THandle<FBuffer> mNormalBuffer = {};
		THandle<FBuffer> mUVBuffer = {};
		THandle<FBuffer> mColorBuffer = {};

		FBounds mBounds;

		uint32 mVertexCount = 0;

		FName mName;
		FAssetHash mAssetHash;
	};

	struct FMeshData final
	{
		FDeviceAddress mIndexBuffer = kNullDeviceAddress;
		FDeviceAddress mPositionBuffer = kNullDeviceAddress;
		FDeviceAddress mNormalBuffer = kNullDeviceAddress;
		FDeviceAddress mUVBuffer = kNullDeviceAddress;

		uint32 mVertexCount = 0;
		uint32 mIndex = 0;
	};

} // Turbo