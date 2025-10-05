#pragma once

#include "CommonMacros.h"
#include "Graphics/Resources.h"
#include "Services/IService.h"

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
	Turbo::FPipelineHandle mComputePipeline;
	Turbo::FDescriptorSetLayoutHandle mComputeSetLayout;
	Turbo::FBufferHandle mComputeUniformBufferHandle;

	Turbo::FPipelineHandle mGraphicsPipeline;
	Turbo::FDescriptorSetLayoutHandle mGraphicsPipelineSetLayout;
	Turbo::FBufferHandle mMeshVertices;
	Turbo::FBufferHandle mMeshIndices;
};