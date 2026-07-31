#pragma once

#include "Core/DataStructures/Handle.h"
#include "Graphics/FrameGraph/RenderGraphHelpers.h"
#include "Graphics/Resources.h"
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

	struct FSceneData final
	{
		uint32 mNumLights = 0;
		uint32 mSceneTLAS = 0;

		uint32 _PADDING[2];
	};

	struct FSceneView
	{
		// Those pointers are valid only during this frame
		FViewData* mViewData = nullptr;
		FSceneData* mSceneData = nullptr;
		FLight* mLights = nullptr; // There is mNumLights in mSceneData;

		FRGResourceHandle mViewDataBufferHandle = {};
		FRGResourceHandle mSceneDataBufferHandle = {};
		FRGResourceHandle mLightsBufferHandle = {};

		// Ray-tracing
		THandle<FTLAS> mTLAS = {};
		FRGResourceHandle mTLASStorageBufferHandle = {};
	};

	struct FDrawIndirectBucket
	{
		THandle<FMaterial> mMaterialHandle = {};
		uint32 mCount = 0;
		FRGResourceHandle mIndirectCommandBuffer = {};
		FRGResourceHandle mDrawBuffer = {};
	};

	class FSceneRenderingLayer : public ILayer
	{
	public:
		virtual void Start() override;
		virtual void Shutdown() override;

		virtual FName GetName() override;

		virtual bool ShouldRender() override;

		void Render(FRenderGraphBuilder& graphBuilder);
		void RenderScene(FRenderGraphBuilder& graphBuilder, FSceneView* SceneView);
		void RenderPostProcess(FRenderGraphBuilder& graphBuilder, FSceneView* SceneView);

	private:
		static void UpdateViewData(FWorld* world, FViewData& viewData);

		static void CreateIndirectRenderBuffers(
			FRenderGraphBuilder& graphBuilder,
			FWorld* world,
			FSceneView* sceneView,
			std::vector<FDrawIndirectBucket>& outBuckets
		);

		static void CreateSceneTLAS(FRenderGraphBuilder& graphBuilder, FWorld* world, FSceneView* sceneView);

	private:
		THandle<FPipeline> mFrustumCullingPipeline = {};
		THandle<FPipeline> mToneMapperPipeline = {};
	};

	template <>
	inline FName GetStaticLayerName<FSceneRenderingLayer>()
	{
		static FName name("SceneRenderingLayer");
		return name;
	}
} // namespace Turbo
