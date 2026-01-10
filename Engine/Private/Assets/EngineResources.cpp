#include "Assets/EngineResources.h"
#include "Assets/MaterialManager.h"

namespace Turbo
{
	namespace
	{
		THandle<FSampler> gDefaultLinearSampler;

		THandle<FTexture> gWhiteTexture;
		THandle<FTexture> gBlackTexture;
	}

	namespace EngineMaterials
	{
		void InitEngineMaterials()
		{
			FMaterialManager& materialManager = entt::locator<FMaterialManager>::value();
			FPipelineBuilder pipelineBuilder = FMaterialManager::CreateOpaquePipeline("MeshTestMaterial.slang");
			materialManager.LoadMaterial<void, void>(kTriangleTest, pipelineBuilder, 0);

			pipelineBuilder = FMaterialManager::CreateOpaquePipeline("OpaqueBasePass.slang");
			const THandle<FMaterial> basePassMat = materialManager.LoadMaterial<FBasePassMaterialData, FBasePassInstanceData>(
				kOpaqueBasePass,
				pipelineBuilder,
				2048
			);

			FGPUDevice& gpu = entt::locator<FGPUDevice>::value();
			gpu.ImmediateSubmit(
				FOnImmediateSubmit::CreateLambda(
					[&](FCommandBuffer& cmd)
					{
						FBasePassMaterialData matData;
						matData.mBaseColorSampler = gDefaultLinearSampler.GetIndex();
						matData.mMetalicRoughnessSampler = gDefaultLinearSampler.GetIndex();
						matData.mNormalSampler = gDefaultLinearSampler.GetIndex();

						materialManager.UpdateMaterialData<FBasePassMaterialData>(cmd, basePassMat, &matData);
					})
			);
		}
	}

	namespace EngineResources
	{
		void InitEngineSamplers()
		{
			FGPUDevice& gpu = entt::locator<FGPUDevice>::value();
			FSamplerBuilder samplerBuilder = {};
			samplerBuilder
				.SetMinMagFilter(vk::Filter::eNearest, vk::Filter::eLinear)
				.SetMipFilter(vk::SamplerMipmapMode::eLinear)
				.SetName(FName("DefaultLinearSampler"));
			gDefaultLinearSampler = gpu.CreateSampler(samplerBuilder);
		}

		void InitEngineTextures()
		{
			FGPUDevice& gpu = entt::locator<FGPUDevice>::value();
			FTextureBuilder textureBuilder = {};
			textureBuilder
				.Init(vk::Format::eR8G8B8A8Srgb, ETextureType::Texture2D)
				.SetNumMips(1)
				.SetSize(glm::uint3(1, 1, 1));

			textureBuilder.SetName(FName("Engine::White"));
			gWhiteTexture = gpu.CreateTexture(textureBuilder);
			constexpr byte whiteBytes[] {0xff_B, 0xff_B, 0xff_B, 0xff_B};
			gpu.UploadTextureUsingStagingBuffer(gWhiteTexture, whiteBytes);

			textureBuilder.SetName(FName("Engine::Black"));
			gBlackTexture = gpu.CreateTexture(textureBuilder);
			constexpr byte blackBytes[] {0x0_B, 0x0_B, 0x0_B, 0xff_B};
			gpu.UploadTextureUsingStagingBuffer( gBlackTexture, blackBytes);
		}

		void DestroyEngineResources()
		{
			FGPUDevice& gpu = entt::locator<FGPUDevice>::value();
			gpu.DestroySampler(gDefaultLinearSampler);

			gpu.DestroyTexture(gWhiteTexture);
			gpu.DestroyTexture(gBlackTexture);
		}

		THandle<FSampler> GetDefaultLinearSampler()
		{
			return gDefaultLinearSampler;
		}

		THandle<FTexture> GetWhiteTexture()
		{
			return gWhiteTexture;
		}

		THandle<FTexture> GetBlackTexture()
		{
			return gBlackTexture;
		}
	}
}

