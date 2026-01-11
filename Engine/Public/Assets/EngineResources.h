#pragma once

#include "MaterialManager.h"
#include "Graphics/GPUDevice.h"

namespace Turbo
{
	struct FMaterial;
	struct FMesh;

	namespace EngineMaterials
	{
		inline const FName kTriangleTest = FName("MeshTriangleTest");
		inline const FName kOpaqueBasePass = FName("OpaqueBasePass");

		void InitEngineMaterials();

		struct FBasePassMaterialData
		{
			uint32 mBaseColorSampler = kInvalidBinding;
			uint32 mMetalicRoughnessSampler = kInvalidBinding;
			uint32 mNormalSampler = kInvalidBinding;
		};

		struct FBasePassInstanceData
		{
			uint32 mBaseColorTexture = kInvalidBinding;
			uint32 mMetalicRoughnessTexture = kInvalidBinding;
			uint32 mNormalTexture = kInvalidBinding;

			float mNormalScale = 1.f;
			glm::float4 mBaseColorFactor = glm::float4(1.f);
			float mMetalicFactor = 0.f;
			float mRoughnessFactor = 1.f;
		};
	}

	namespace EngineResources
	{
		void InitEngineSamplers();
		void InitEngineTextures();
		void LoadPlaceholders();
		void LoadPlaceholderMesh();

		void DestroyEngineResources();

		THandle<FSampler> GetDefaultLinearSampler();

		THandle<FTexture> GetWhiteTexture();
		THandle<FTexture> GetBlackTexture();
		THandle<FTexture> GetPlaceholderTexture();

		THandle<FMesh> GetPlaceholderMesh();

		THandle<FMaterial> GetPlaceholderMaterial();

		void GenerateCheckerboardTextureData(
			byte* outBytes,
			glm::uint2 size,
			std::span<const byte> onValue,
			std::span<const byte> offValue
		);
	}
}
