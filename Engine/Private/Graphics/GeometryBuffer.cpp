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

		const FRGTextureInfo colorInfo = {
			.mWidth = static_cast<uint16>(resolution.x),
			.mHeight = static_cast<uint16>(resolution.y),
			.mFormat = kColorFormat,
			.mFlags = ETextureFlags::RenderTarget | ETextureFlags::StorageImage,
			.mName = geometryBufferColorName
		};

		const FRGTextureInfo depthInfo = {
			.mWidth = static_cast<uint16>(resolution.x),
			.mHeight = static_cast<uint16>(resolution.y),
			.mFormat = kDepthStencilFormat,
			.mFlags = ETextureFlags::RenderTarget,
			.mName = geometryBufferDepthName
		};

		mColor = graphBuilder.CreateTexture(colorInfo);
		mDepth = graphBuilder.CreateTexture(depthInfo);
	}

	void FGeometryBuffer::BlitToPresent(FRenderGraphBuilder& graphBuilder, FRGResourceHandle presentTexture) const
	{
		RenderGraphUtils::AddBlitTexturePass(graphBuilder, mColor, presentTexture);
	}
} // Turbo