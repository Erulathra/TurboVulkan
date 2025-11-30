#pragma once

#include "ILayer.h"
#include "World/Camera.h"
#include "World/World.h"

namespace Turbo
{
	class FBuffer;
	struct FMaterial;

	// Replace with growable buffer
	constexpr size_t kNumAllocatedMaterialInstances = 512;

	class FSceneRenderingLayer : public ILayer
	{
	public:
		virtual void Start() override;

		virtual void RenderScene(FGPUDevice* gpu, FCommandBuffer* cmd) override;
		virtual bool ShouldRender() override;

	private:
		void UpdateViewData(FGPUDevice* gpu, FCommandBuffer* cmd, FWorld* world);

	private:
		FViewData mViewData;
		THandle<FBuffer> mViewDataUniformBuffer;

		std::vector<THandle<FMaterial>> mMaterials;
	};
} // Turbo