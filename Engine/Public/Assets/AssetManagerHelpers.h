#pragma once

namespace Turbo
{
	using FAssetHash = uint32;

	// Replace with something more robust
	constexpr uint32 kMaxMeshes = 1024;

	struct FMeshLoadSettings
	{
		uint32 mMeshIndex = 0;
		uint32 mSubMeshIndex = 0;
		bool mbLevelAsset = true;
	};

	struct FTextureAsset
	{
		FAssetHash mAssetHash;
	};

	struct FTextureLoadingSettings
	{
		bool mbSRGB : 1 = true;
		bool mbLevelAsset : 1 = true;
	};
}