#pragma once
#include "RenderGraph.h"

namespace Turbo
{
	namespace RenderGraphUtils
	{
		void AddClearTexturePass(FRenderGraphBuilder& graphBuilder, FRGResourceHandle texture, glm::float4 color);
		void AddBlitTexturePass(FRenderGraphBuilder& graphBuilder, FRGResourceHandle srcTexture, FRGResourceHandle dstTexture);

		void AddFillBufferPass(
			FRenderGraphBuilder& graphBuilder,
			FRGResourceHandle srcBuffer,
			FDeviceSize offset,
			FDeviceSize size,
			uint32 value
		);
	};
} // Turbo
