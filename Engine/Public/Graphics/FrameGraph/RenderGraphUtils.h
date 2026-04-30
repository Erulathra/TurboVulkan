#pragma once
#include "RenderGraph.h"

namespace Turbo
{
	namespace RenderGraphUtils
	{
		void AddClearTexturePass(FRenderGraphBuilder& graphBuilder, FRGResourceHandle texture, glm::float4 color);
		void AddBlitTexturePass(FRenderGraphBuilder& graphBuilder, FRGResourceHandle srcTexture, FRGResourceHandle dstTexture);
	};
} // Turbo
