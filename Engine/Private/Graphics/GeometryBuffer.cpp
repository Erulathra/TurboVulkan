#include "Graphics/GeometryBuffer.h"

#include "Graphics/Enums.h"
#include "Graphics/GPUDevice.h"

namespace Turbo
{
	void FGeometryBuffer::Init(FRenderGraphBuilder& graphBuilder, glm::ivec2 resolution)
	{
		const static FName geometryBufferColorName = FName{"GBuffer_Color"};
		const static FName geometryBufferDepthName = FName{"GBuffer_Depth"};

		const FRGTextureInfo colorInfo = {
			.mWidth = static_cast<uint16>(resolution.x),
			.mHeight = static_cast<uint16>(resolution.y),
			.mFormat = kColorFormat,
			.mName = geometryBufferColorName
		};

		const FRGTextureInfo depthInfo = {
			.mWidth = static_cast<uint16>(resolution.x),
			.mHeight = static_cast<uint16>(resolution.y),
			.mFormat = kDepthStencilFormat,
			.mName = geometryBufferDepthName
		};

		mColor = graphBuilder.AddTexture(colorInfo);
		mDepth = graphBuilder.AddTexture(depthInfo);
	}

	void FGeometryBuffer::BlitToPresent(FRenderGraphBuilder& graphBuilder, FRGResourceHandle presentTexture) const
	{
		const static FName passName("BlitGeometryBufferToPresentTexture");
		graphBuilder.AddPass(
			passName,
			FRGSetupPassDelegate::CreateLambda(
				[&](FRGPassInfo& passInfo)
				{
					passInfo.mPassType = EPassType::Transfer;
					passInfo.ReadTexture(mColor);
					passInfo.WriteTexture(presentTexture);
				}),
			FRGExecutePassDelegate::CreateLambda(
				[colorRes = mColor, presentRes = presentTexture](FGPUDevice& gpu, FCommandBuffer& cmd, FRenderResources& resources)
				{
					const THandle<FTexture> colorHandle = resources.mTextures[colorRes];
					const THandle<FTexture> presentHandle = resources.mTextures[presentRes];

					const FTextureCold* colorTexCold = gpu.AccessTextureCold(colorHandle);
					const FTextureCold* presentTexCold = gpu.AccessTextureCold(presentHandle);

					const FRect2DInt srcRect = FRect2DInt::FromSize(colorTexCold->GetSize2D());
					const FRect2DInt dstRect = FRect2DInt::FromSize(presentTexCold->GetSize2D());

					cmd.BlitImage(colorHandle, srcRect, presentHandle, dstRect);
				})
		);
	}
} // Turbo