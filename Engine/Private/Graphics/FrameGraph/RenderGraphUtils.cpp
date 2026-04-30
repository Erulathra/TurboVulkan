#include "Graphics/FrameGraph/RenderGraphUtils.h"

#include "Graphics/GPUDevice.h"

namespace Turbo
{
	void RenderGraphUtils::AddClearTexturePass(FRenderGraphBuilder& graphBuilder, FRGResourceHandle texture, glm::float4 color)
	{
		const FName passName(fmt::format("Clear {} to {}", graphBuilder.GetTextureInfo(texture).mName, color));
		FRGPassInitializer pass = graphBuilder.AddPass(passName, EPassType::Transfer);
		pass->WriteTexture(texture);

		pass->mExecutePass.BindLambda(
			[texture, color](FGPUDevice& gpu, FCommandBuffer& cmd, FRenderResources& resources)
			{
				const THandle<FTexture> handle = resources.mTextures[texture];
				cmd.ClearImage(handle, color, vk::ImageLayout::eTransferDstOptimal);
			}
		);
	}

	void RenderGraphUtils::AddBlitTexturePass(FRenderGraphBuilder& graphBuilder, FRGResourceHandle srcTexture, FRGResourceHandle dstTexture)
	{
		TURBO_CHECK(srcTexture != dstTexture)

		const FName passName(fmt::format(
				"Blit {} to {}",
				graphBuilder.GetTextureInfo(srcTexture).mName,
				graphBuilder.GetTextureInfo(dstTexture).mName)
		);

		FRGPassInitializer pass = graphBuilder.AddPass(passName, EPassType::Transfer);
		pass->ReadTexture(srcTexture);
		pass->WriteTexture(dstTexture);

		pass->mExecutePass.BindLambda(
			[srcTexture, dstTexture](FGPUDevice& gpu, FCommandBuffer& cmd, FRenderResources& resources)
			{
				const THandle<FTexture> srcHandle = resources.mTextures[srcTexture];
				const THandle<FTexture> dstHandle = resources.mTextures[dstTexture];

				const FTextureCold* colorTexCold = gpu.AccessTextureCold(srcHandle);
				const FTextureCold* presentTexCold = gpu.AccessTextureCold(dstHandle);

				const FRect2DInt srcRect = FRect2DInt::FromSize(colorTexCold->GetSize2D());
				const FRect2DInt dstRect = FRect2DInt::FromSize(presentTexCold->GetSize2D());

				cmd.BlitImage(srcHandle, srcRect, dstHandle, dstRect);
			}
		);
	}
} // Turbo