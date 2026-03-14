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

	class FSceneRenderingLayer : public ILayer
	{
	public:
		virtual void Start() override;
		virtual void Shutdown() override;

		virtual FName GetName() override
		{
			static FName name("Scene Rendering Layer");
			return name;
		}

		virtual void RenderScene(FRenderGraphBuilder& graphBuilder) override;
		virtual bool ShouldRender() override;

	private:
		static void UpdateViewData(FWorld* world, FViewData& viewData);
		static void RenderMeshes(
			FGPUDevice& gpu,
			FCommandBuffer& cmd,
			FWorld* world,
			const FViewData& viewData,
			THandle<FBuffer> viewDataBuffer
		);

		static void CreateIndirectRenderBuffers(
			FRenderGraphBuilder& graphBuilder,
			FWorld* world,
			FViewData& viewData,
			std::vector<FDrawIndirectBucket>& outBuckets
		);

		static void DrawIndirect(
			FGPUDevice& gpu,
			FCommandBuffer& cmd,
			FRenderResources& resources,
			FRGResourceHandle viewDataBufferHandle,
			const std::vector<FDrawIndirectBucket>& buckets
		);
	};
} // Turbo