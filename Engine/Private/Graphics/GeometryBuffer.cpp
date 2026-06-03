#include "Graphics/GeometryBuffer.h"

#include "Graphics/Enums.h"
#include "Graphics/GPUDevice.h"
#include "Graphics/FrameGraph/RenderGraphUtils.h"

namespace Turbo
{
	void FGeometryBuffer::Init(FRenderGraphBuilder& graphBuilder, glm::ivec2 resolution)
	{
		const static FName geometryBufferColorName = FName{"GBuffer_Color"};
		const static FName geometryBufferDepthName = FName{"GBuffer_Depth"};
		const static FName geometryBufferAfterToneMapName = FName{"GBuffer_AfterToneMap"};

		const FRGTextureInfo colorInfo = {
			.mWidth = static_cast<uint16>(resolution.x),
			.mHeight = static_cast<uint16>(resolution.y),
			.mFormat = kColorFormat,
			.mFlags = ETextureFlags::RenderTarget,
			.mName = geometryBufferColorName
		};

		const FRGTextureInfo depthInfo = {
			.mWidth = static_cast<uint16>(resolution.x),
			.mHeight = static_cast<uint16>(resolution.y),
			.mFormat = kDepthStencilFormat,
			.mFlags = ETextureFlags::RenderTarget,
			.mName = geometryBufferDepthName
		};

		const FRGTextureInfo afterToneMap = {
			.mWidth = static_cast<uint16>(resolution.x),
			.mHeight = static_cast<uint16>(resolution.y),
			.mFormat = kAfterToneMapFormat,
			.mFlags = ETextureFlags::StorageImage,
			.mName = geometryBufferAfterToneMapName
		};

		mSceneColor = graphBuilder.CreateTexture(colorInfo);
		mDepthStencil = graphBuilder.CreateTexture(depthInfo);
		mAfterToneMap = graphBuilder.CreateTexture(afterToneMap);
	}

	void FGeometryBuffer::BlitToPresent(FRenderGraphBuilder& graphBuilder, FRGResourceHandle presentTexture) const
	{
		RenderGraphUtils::AddBlitTexturePass(graphBuilder, mAfterToneMap, presentTexture);
	}
} // Turbo