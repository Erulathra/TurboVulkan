#include "Services/SceneRenderingLayer.h"

#include "Core/CoreTimer.h"
#include "Core/Engine.h"
#include "Graphics/GPUDevice.h"
#include "World/Camera.h"
#include "Graphics/ResourceBuilders.h"
#include "World/World.h"

namespace Turbo
{
	struct FSceneRenderingPushConstants
	{
		glm::float4x4 mModelToProj;
		glm::float3x3 mInvModelToProj;

		DeviceAddress mViewData;
		DeviceAddress mMaterialInstance;
		DeviceAddress mMeshData;
	};

	void FSceneRenderingLayer::Start()
	{
		FBufferBuilder viewDataBufferBuilder;
		viewDataBufferBuilder.Init(
			vk::BufferUsageFlagBits::eUniformBuffer,
			EBufferFlags::CreateMapped,
			sizeof(FViewData)
		);
		viewDataBufferBuilder.SetName(FName("ViewData_Uniform"));

		FGPUDevice& gpuDevice = entt::locator<FGPUDevice>::value();
		mViewDataUniformBuffer = gpuDevice.CreateBuffer(viewDataBufferBuilder);
	}

	void FSceneRenderingLayer::UpdateViewData(FGPUDevice* gpu, FCommandBuffer* cmd, FWorld* world)
	{
		FCameraUtils::UpdateDirtyCameras(world->mRegistry);

		auto mainCameraView = world->mRegistry.view<FCameraCache const, FWorldTransform const, FMainViewport const>();
		TURBO_CHECK(mainCameraView.begin() != mainCameraView.end())

		const entt::entity mainCameraEntity = *mainCameraView.begin();
		const FCameraCache& cameraCache = mainCameraView.get<FCameraCache>(mainCameraEntity);
		const FWorldTransform& cameraTransform = mainCameraView.get<FWorldTransform>(mainCameraEntity);

		FViewData viewData = {};
		viewData.mProjectionMatrix = cameraCache.mProjectionMatrix;
		viewData.mViewMatrix = cameraTransform.mTransform;
		viewData.mWorldToProjection = viewData.mProjectionMatrix * viewData.mViewMatrix;

		const FCoreTimer& coreTimer = entt::locator<FCoreTimer>::value();
		viewData.mTime = coreTimer.TimeFromEngineStart();
		viewData.mWorldTime = coreTimer.TimeFromEngineStart();
		viewData.mDeltaTime = coreTimer.DeltaTime();
		viewData.mFrameIndex = gpu->GetNumRenderedFrames();

		FBuffer* viewDataBuffer = gpu->AccessBuffer(mViewDataUniformBuffer);
		std::memcpy(viewDataBuffer->GetMappedAddress(), &viewData, sizeof(FViewData));
		cmd->BufferBarrier(
			mViewDataUniformBuffer,
			vk::AccessFlagBits::eHostWrite,
			vk::PipelineStageFlagBits::eHost,
			vk::AccessFlagBits::eShaderRead,
			vk::PipelineStageFlagBits::eTransfer
		);
	}

	void FSceneRenderingLayer::RenderScene(FGPUDevice* gpu, FCommandBuffer* cmd)
	{
		TRACE_ZONE_SCOPED_N("Render Scene")

		FWorld* world = gEngine->GetWorld();
		UpdateViewData(gpu, cmd, world);


	}

	bool FSceneRenderingLayer::ShouldRender()
	{
		return true;
	}
} // Turbo