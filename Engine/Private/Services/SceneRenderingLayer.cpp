#include "Services/SceneRenderingLayer.h"

#include "Assets/AssetManager.h"
#include "Core/CoreTimer.h"
#include "Core/Engine.h"
#include "Graphics/GeometryBuffer.h"
#include "Graphics/GPUDevice.h"
#include "Graphics/ResourceBuilders.h"
#include "World/Camera.h"
#include "World/MeshComponent.h"
#include "World/World.h"

namespace Turbo
{
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

	void FSceneRenderingLayer::Shutdown()
	{
		FGPUDevice& gpu = entt::locator<FGPUDevice>::value();
		gpu.DestroyBuffer(mViewDataUniformBuffer);
	}

	void FSceneRenderingLayer::UpdateViewData(FGPUDevice* gpu, FCommandBuffer* cmd, FWorld* world)
	{
		TRACE_ZONE_SCOPED()

		FCameraUtils::UpdateDirtyCameras(world->mRegistry);

		auto mainCameraView = world->mRegistry.view<FCameraCache const, FWorldTransform const, FMainViewport const>();
		TURBO_CHECK(mainCameraView.begin() != mainCameraView.end())

		const entt::entity mainCameraEntity = *mainCameraView.begin();
		const FCameraCache& cameraCache = mainCameraView.get<FCameraCache>(mainCameraEntity);
		const FWorldTransform& cameraTransform = mainCameraView.get<FWorldTransform>(mainCameraEntity);

		mViewData.mProjectionMatrix = cameraCache.mProjectionMatrix;
		mViewData.mViewMatrix = cameraTransform.mTransform;
		mViewData.mWorldToProjection = mViewData.mProjectionMatrix * mViewData.mViewMatrix;

		mViewData.mTime = FCoreTimer::TimeFromEngineStart();
		mViewData.mWorldTime = FCoreTimer::TimeFromEngineStart();
		mViewData.mDeltaTime = FCoreTimer::DeltaTime();
		mViewData.mFrameIndex = gpu->GetNumRenderedFrames();

		FBuffer* viewDataBuffer = gpu->AccessBuffer(mViewDataUniformBuffer);
		std::memcpy(viewDataBuffer->GetMappedAddress(), &mViewData, sizeof(FViewData));
		cmd->BufferBarrier(
			mViewDataUniformBuffer,
			vk::AccessFlagBits2::eHostWrite,
			vk::PipelineStageFlagBits2::eHost,
			vk::AccessFlagBits2::eShaderRead,
			vk::PipelineStageFlagBits2::eVertexShader
		);
	}

	void FSceneRenderingLayer::RenderMeshes(FGPUDevice* gpu, FCommandBuffer* cmd, FWorld* world)
	{
		TRACE_ZONE_SCOPED()

		{
			TRACE_ZONE_SCOPED_N("Sort drawcalls/entities")
			world->mRegistry.sort<FMeshComponent>(
				[]( const FMeshComponent& lhs, const FMeshComponent& rhs) -> bool
				{
					if (lhs.mMaterial.mIndex == rhs.mMaterial.mIndex)
					{
						if (lhs.mMaterialInstance.mIndex == rhs.mMaterialInstance.mIndex)
						{
							return lhs.mMesh.mIndex < rhs.mMesh.mIndex;
						}

						return lhs.mMaterialInstance.mIndex < rhs.mMaterialInstance.mIndex;
					}

					return lhs.mMaterial.mIndex < rhs.mMaterial.mIndex;
				});
		}

		{
			TRACE_ZONE_SCOPED_N("Render meshes")

			const FGeometryBuffer& geometryBuffer = entt::locator<FGeometryBuffer>::value();

			const THandle<FTexture> drawImageHandle = geometryBuffer.GetColor();
			const THandle<FTexture> dephtImageHandle = geometryBuffer.GetDepth();

			cmd->TransitionImage(drawImageHandle, vk::ImageLayout::eColorAttachmentOptimal);
			cmd->TransitionImage(dephtImageHandle, vk::ImageLayout::eDepthAttachmentOptimal);

			FRenderingAttachments renderingAttachments;
			renderingAttachments.AddColorAttachment(drawImageHandle);
			renderingAttachments.SetDepthAttachment(dephtImageHandle);

			cmd->BeginRendering(renderingAttachments);
			cmd->SetViewport(FViewport::FromSize(geometryBuffer.GetResolution()));
			cmd->SetScissor(FRect2DInt::FromSize(geometryBuffer.GetResolution()));

			const auto meshTransformView = world->mRegistry.view<FMeshComponent, FWorldTransform>();

			const FAssetManager& assetManager = entt::locator<FAssetManager>::value();
			const FMaterialManager& materialManager = entt::locator<FMaterialManager>::value();

			THandle<FMaterial> materialHandle;
			THandle<FMaterial::Instance> materialInstanceHandle;
			THandle<FMesh> meshHandle;
			const FMesh* mesh = {};

			FDeviceAddress viewDataDeviceAddress = gpu->AccessBuffer(mViewDataUniformBuffer)->GetDeviceAddress();

			for (const entt::entity& entity : meshTransformView)
			{
				const FWorldTransform& worldTransform = meshTransformView.get<FWorldTransform>(entity);
				const FMeshComponent& meshComponent = meshTransformView.get<FMeshComponent>(entity);

				if (meshComponent.mMaterial != materialHandle)
				{
					materialHandle = meshComponent.mMaterial;
					const FMaterial* material = materialManager.AccessMaterial(materialHandle);

					cmd->BindPipeline(material->mPipeline);
					cmd->BindDescriptorSet(gpu->GetBindlessResourcesSet(), 0);
				}

				if (meshComponent.mMaterialInstance != materialInstanceHandle)
				{
					materialInstanceHandle = meshComponent.mMaterialInstance;
					const FMaterial::Instance* materialInstance = materialManager.AccessInstance(materialInstanceHandle);
					TURBO_CHECK(materialInstance->material == materialHandle)
				}

				if (meshComponent.mMesh != meshHandle)
				{
					meshHandle = meshComponent.mMesh;
					mesh = assetManager.AccessMesh(meshHandle);

					cmd->BindIndexBuffer(mesh->mIndicesBuffer);
				}

				FMaterial::PushConstants pushConstants = {};
				pushConstants.mModelToProj = mViewData.mWorldToProjection * worldTransform.mTransform;

				const glm::float3x3 invModelToWorld =  glm::float3x3(glm::transpose(glm::inverse(worldTransform.mTransform)));
				pushConstants.mInvModelToWorld[0] = glm::float4(invModelToWorld[0], 0.f);
				pushConstants.mInvModelToWorld[1] = glm::float4(invModelToWorld[1], 0.f);
				pushConstants.mInvModelToWorld[2] = glm::float4(invModelToWorld[2], 0.f);

				pushConstants.mViewData = viewDataDeviceAddress;
				pushConstants.mMaterialInstance = materialManager.GetMaterialInstanceAddress(*gpu, materialInstanceHandle);
				pushConstants.mMeshData = assetManager.GetMeshPointersAddress(*gpu, meshHandle);
				pushConstants.mSceneData = kNullDeviceAddress;

				if (mesh != nullptr)
				{
					cmd->PushConstants(pushConstants);
					cmd->DrawIndexed(mesh->mVertexCount);
				}
			}

			cmd->EndRendering();
		}
	}

	void FSceneRenderingLayer::RenderScene(FGPUDevice* gpu, FCommandBuffer* cmd)
	{
		TRACE_ZONE_SCOPED_N("Render Scene")

		FWorld* world = gEngine->GetWorld();
		world->UpdateWorldTransforms();

		auto mainCameraView = world->mRegistry.view<FCamera>();
		if (mainCameraView.begin() == mainCameraView.end())
		{
			TURBO_LOG(LOG_SCENE_RENDERING, Error, "Scene doesn't contain any camera.")
			return;
		}

		UpdateViewData(gpu, cmd, world);
		RenderMeshes(gpu, cmd, world);
	}

	bool FSceneRenderingLayer::ShouldRender()
	{
		return true;
	}
} // Turbo