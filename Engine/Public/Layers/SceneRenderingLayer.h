#pragma once

#include "Layer.h"
#include "World/Camera.h"
#include "World/World.h"

DECLARE_LOG_CATEGORY(LogSceneRendering, Display, Display)

namespace Turbo
{
	struct FBuffer;
	struct FMaterial;
	class FCommandBuffer;

	// Replace with growable buffer
	constexpr size_t kNumAllocatedMaterialInstances = 512;

	struct FDrawIndirectBucket
	{
		THandle<FMaterial> mMaterialHandle = {};
		uint32 mCount = 0;
		FRGResourceHandle mIndirectCommandBuffer = {};
		FRGResourceHandle mDrawBuffer = {};
	};

	struct FSceneData final
	{
		uint32 mNumLights = 0;
		uint32 _PADDING[3];
	};

	class FSceneRenderingLayer : public ILayer
	{
	public:
		virtual void Start() override;
		virtual void Shutdown() override;

		virtual FName GetName() override;

		virtual void RenderScene(FRenderGraphBuilder& graphBuilder) override;
		virtual bool ShouldRender() override;

	private:
		static void UpdateViewData(FWorld* world, FViewData& viewData);

		static void CreateIndirectRenderBuffers(
			FRenderGraphBuilder& graphBuilder,
			FWorld* world,
			FViewData& viewData,
			std::vector<FDrawIndirectBucket>& outBuckets
		);

	private:
		THandle<FPipeline> mFrustumCullingPipeline = {};
	};
} // Turbo
