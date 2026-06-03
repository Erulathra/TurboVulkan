#pragma once

#include "FrameGraph/RenderGraph.h"

namespace Turbo
{
	class FCommandBuffer;

	class FGeometryBuffer
	{
	public:
		static constexpr vk::Format kColorFormat = vk::Format::eB10G11R11UfloatPack32;
		static constexpr vk::Format kDepthStencilFormat = vk::Format::eD32Sfloat;

		static constexpr vk::Format kAfterToneMapFormat = vk::Format::eR8G8B8A8Unorm;

	public:
		void Init(FRenderGraphBuilder& graphBuilder, glm::ivec2 resolution);
		void BlitToPresent(FRenderGraphBuilder& graphBuilder, FRGResourceHandle presentTexture) const;

	public:
		FRGResourceHandle mDepthStencil = {};
		FRGResourceHandle mSceneColor = {};
		FRGResourceHandle mAfterToneMap = {};
	};
} // Turbo