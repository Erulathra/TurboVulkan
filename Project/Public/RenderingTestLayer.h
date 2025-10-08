#pragma once

#include "CommonMacros.h"
#include "Graphics/Resources.h"
#include "Services/ILayer.h"

class FRenderingTestLayer final : public Turbo::ILayer
{
	/** IService Interface */
public:
	virtual void Start() override;
	virtual void Shutdown() override;

	virtual void BeginTick_GameThread(float deltaTime) override;
	virtual void PostBeginFrame_RenderThread(Turbo::FGPUDevice* gpu, Turbo::FCommandBuffer* cmd) override;

	virtual bool ShouldTick() override { return true; }
	virtual bool ShouldRender() override { return true; }

	virtual Turbo::FName GetName() override;

	/** IService Interface end */

private:
	Turbo::THandle<Turbo::FPipeline> mComputePipeline;
	Turbo::THandle<Turbo::FDescriptorSetLayout> mComputeSetLayout;
	Turbo::THandle<Turbo::FBuffer> mComputeUniformBufferHandle;

	Turbo::THandle<Turbo::FPipeline> mGraphicsPipeline;
	Turbo::THandle<Turbo::FDescriptorSetLayout> mGraphicsPipelineSetLayout;
	Turbo::THandle<Turbo::FBuffer> mMeshVertices;
	Turbo::THandle<Turbo::FBuffer> mMeshIndices;
};