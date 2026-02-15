#pragma once

#include "FrameGraph/FrameGraph.h"
#include "Graphics/Resources.h"

namespace Turbo
{
	class FCommandBuffer;

	class FGeometryBuffer
	{
	public:
		static constexpr vk::Format kColorFormat = vk::Format::eR16G16B16A16Sfloat;
		static constexpr vk::Format kDepthStencilFormat = vk::Format::eD32Sfloat;

	public:
		void Init(FRenderGraphBuilder& graphBuilder, glm::ivec2 resolution);
		void BlitToPresent(FRenderGraphBuilder& graphBuilder, FRGResourceHandle presentTexture) const;

	public:
		FRGResourceHandle mColor = {};
		FRGResourceHandle mDepth = {};
	};
} // Turbo