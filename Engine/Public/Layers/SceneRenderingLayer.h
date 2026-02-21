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
		void UpdateViewData(FGPUDevice& gpu, FCommandBuffer& cmd, FWorld* world, FViewData& viewData);
		void RenderMeshes(FGPUDevice& gpu, FCommandBuffer& cmd, FWorld* world, const FViewData& viewData);

	private:
		THandle<FBuffer> mViewDataUniformBuffer;
	};
} // Turbo