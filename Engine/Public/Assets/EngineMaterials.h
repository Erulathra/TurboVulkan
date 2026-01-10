#pragma once

namespace Turbo::EngineMaterials
{
	inline const FName kTriangleTest = FName("MeshTriangleTest");
	inline const FName kOpaqueBasePass = FName("OpaqueBasePass");

	void InitEngineMaterials();

	struct FBasePassMaterialData
	{
		uint32 mBaseColorSampler;
		uint32 mMetalicRoughnessSampler;
		uint32 mNormalSampler;
	};

	struct FBasePassInstanceData
	{
		uint32 mBaseColorTexture;
		uint32 mMetalicRoughnessTexture;
		uint32 mNormalTexture;
		float mNormalScale;
	};
}
