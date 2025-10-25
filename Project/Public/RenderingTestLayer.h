#pragma once

#include "CommonMacros.h"
#include "Assets/StaticMesh.h"
#include "Core/DataStructures/Pool.h"
#include "Graphics/Resources.h"
#include "Services/ILayer.h"

class FRenderingTestLayer final : public Turbo::ILayer
{
	/** IService Interface */
public:
	virtual ~FRenderingTestLayer() override;
	FRenderingTestLayer();

	virtual void Start() override;
	virtual void Shutdown() override;

	virtual void BeginTick_GameThread(double deltaTime) override;
	virtual void PostBeginFrame_RenderThread(Turbo::FGPUDevice* gpu, Turbo::FCommandBuffer* cmd) override;

	virtual bool ShouldTick() override { return true; }
	virtual bool ShouldRender() override { return true; }

	virtual Turbo::FName GetName() override;

	/** IService Interface end */

private:
	glm::vec3 mModelLocation = {0.f, 0.f, 0.f};
	glm::vec3 mCameraLocation = {0.f, 0.f, 3.f};
	glm::vec2 mCameraRotation = {0.f, 0.f};
	float mCameraFov = 60.f;
	glm::vec2 mCameraNearFar = {0.1f, 1000.f};

	Turbo::THandle<Turbo::FPipeline> mGraphicsPipeline;

	Turbo::THandle<Turbo::FSubMesh> mMeshHandle;
};
