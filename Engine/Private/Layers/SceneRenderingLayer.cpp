#include "Layers/SceneRenderingLayer.h"

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

	void FSceneRenderingLayer::UpdateViewData(FGPUDevice& gpu, FCommandBuffer& cmd, FWorld* world)
	{
		TRACE_ZONE_SCOPED()

		FCameraUtils::UpdateDirtyCameras(world->mRegistry);

		auto mainCameraView = world->mRegistry.view<FCameraCache const, FWorldTransform const, FMainViewport const>();
		TURBO_CHECK(mainCameraView.begin() != mainCameraView.end())

		const entt::entity mainCameraEntity = *mainCameraView.begin();
		const FCameraCache& cameraCache = mainCameraView.get<FCameraCache>(mainCameraEntity);
		const FWorldTransform& cameraTransform = mainCameraView.get<FWorldTransform>(mainCameraEntity);

		mViewData.mProjectionMatrix = cameraCache.mProjectionMatrix;
		mViewData.mViewMatrix = glm::inverse(cameraTransform.mTransform);
		mViewData.mWorldToProjection = mViewData.mProjectionMatrix * mViewData.mViewMatrix;

		mViewData.mTime = FCoreTimer::TimeFromEngineStart();
		mViewData.mWorldTime = FCoreTimer::TimeFromEngineStart();
		mViewData.mDeltaTime = FCoreTimer::DeltaTime();
		mViewData.mFrameIndex = gpu.GetNumRenderedFrames();

		FBuffer* viewDataBuffer = gpu.AccessBuffer(mViewDataUniformBuffer);
		std::memcpy(viewDataBuffer->GetMappedAddress(), &mViewData, sizeof(FViewData));
		cmd.BufferBarrier(
			mViewDataUniformBuffer,
			vk::AccessFlagBits2::eHostWrite,
			vk::PipelineStageFlagBits2::eHost,
			vk::AccessFlagBits2::eShaderRead,
			vk::PipelineStageFlagBits2::eVertexShader
		);
	}

	void FSceneRenderingLayer::RenderMeshes(FGPUDevice& gpu, FCommandBuffer& cmd, FWorld* world)
	{
		TRACE_ZONE_SCOPED()

		{
			TRACE_ZONE_SCOPED_N("Sort drawcalls/entities")
			world->mRegistry.sort<FMeshComponent>(
				[]( const FMeshComponent& lhs, const FMeshComponent& rhs) -> bool
				{
					if (lhs.mMaterial.GetIndex() == rhs.mMaterial.GetIndex())
					{
						if (lhs.mMaterialInstance.GetIndex() == rhs.mMaterialInstance.GetIndex())
						{
							return lhs.mMesh.GetIndex() < rhs.mMesh.GetIndex();
						}

						return lhs.mMaterialInstance.GetIndex() < rhs.mMaterialInstance.GetIndex();
					}

					return lhs.mMaterial.GetIndex() < rhs.mMaterial.GetIndex();
				});
		}

		{
			TRACE_ZONE_SCOPED_N("Render meshes")

			const FGeometryBuffer& geometryBuffer = entt::locator<FGeometryBuffer>::value();

			const THandle<FTexture> drawImageHandle = geometryBuffer.GetColor();
			const THandle<FTexture> dephtImageHandle = geometryBuffer.GetDepth();

			cmd.TransitionImage(drawImageHandle, vk::ImageLayout::eColorAttachmentOptimal);
			cmd.TransitionImage(dephtImageHandle, vk::ImageLayout::eDepthAttachmentOptimal);

			FRenderingAttachments renderingAttachments;
			renderingAttachments.AddColorAttachment(drawImageHandle);
			renderingAttachments.SetDepthAttachment(dephtImageHandle);

			cmd.BeginRendering(renderingAttachments);
			cmd.SetViewport(FViewport::FromSize(geometryBuffer.GetResolution()));
			cmd.SetScissor(FRect2DInt::FromSize(geometryBuffer.GetResolution()));

			const auto meshTransformView = world->mRegistry.view<FMeshComponent, FWorldTransform>();

			const FAssetManager& assetManager = entt::locator<FAssetManager>::value();
			const FMaterialManager& materialManager = entt::locator<FMaterialManager>::value();

			THandle<FMaterial> materialHandle;
			THandle<FMaterial::Instance> materialInstanceHandle;
			THandle<FMesh> meshHandle;
			const FMesh* mesh = {};

			FDeviceAddress viewDataDeviceAddress = gpu.AccessBuffer(mViewDataUniformBuffer)->GetDeviceAddress();

#if WITH_PROFILER
			FCounterType numDrawCalls = 0;
			FCounterType numPipelineSwitches = 0;
#endif // WITH_PROFILER
			for (const entt::entity& entity : meshTransformView)
			{
				TRACE_ZONE_SCOPED_N("Render entity")
				TRACE_GPU_SCOPED(gpu, cmd, "Render entity");

				const FWorldTransform& worldTransform = meshTransformView.get<FWorldTransform>(entity);
				const FMeshComponent& meshComponent = meshTransformView.get<FMeshComponent>(entity);

				if (meshComponent.mMaterial != materialHandle)
				{
					materialHandle = meshComponent.mMaterial;
					const FMaterial* material = materialManager.AccessMaterial(materialHandle);

					cmd.BindPipeline(material->mPipeline);
					cmd.BindDescriptorSet(gpu.GetBindlessResourcesSet(), 0);

#if WITH_PROFILER
					numPipelineSwitches++;
#endif // WITH_PROFILER
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

					cmd.BindIndexBuffer(mesh->mIndicesBuffer);
				}

				FMaterial::PushConstants pushConstants = {};
				pushConstants.mModelToProj = mViewData.mWorldToProjection * worldTransform.mTransform;
				pushConstants.mModelToView = mViewData.mViewMatrix * worldTransform.mTransform;
				pushConstants.mInvModelToView = glm::float3x3(glm::transpose(glm::inverse(pushConstants.mModelToView)));

				pushConstants.mViewData = viewDataDeviceAddress;
				pushConstants.mMaterialInstance = materialManager.GetMaterialInstanceAddress(gpu, materialInstanceHandle);
				pushConstants.mMeshData = assetManager.GetMeshPointersAddress(gpu, meshHandle);
				pushConstants.mSceneData = kNullDeviceAddress;

				if (mesh != nullptr)
				{
					TRACE_ZONE_SCOPED_N("Draw Indexed")

					cmd.PushConstants(pushConstants);
					cmd.DrawIndexed(mesh->mVertexCount);

#if WITH_PROFILER
					numDrawCalls++;
#endif // WITH_PROFILER
				}
			}

			cmd.EndRendering();

			static const cstring kDrawCallPlotName = "Draw calls";
			TRACE_PLOT_CONFIGURE(kDrawCallPlotName, EPlotFormat::Number, true, true, 0xFF00FF)
			TRACE_PLOT(kDrawCallPlotName, numDrawCalls)

			static const cstring kPipelineSwitchesName = "Pipeline switches";
			TRACE_PLOT_CONFIGURE(kPipelineSwitchesName, EPlotFormat::Number, true, true, 0xFFFF00)
			TRACE_PLOT(kPipelineSwitchesName, numPipelineSwitches)
		}

	}

	void FSceneRenderingLayer::RenderScene(FGPUDevice& gpu, FCommandBuffer& cmd)
	{
		TRACE_ZONE_SCOPED_N("Render Scene")

		FWorld* world = gEngine->GetWorld();
		world->UpdateWorldTransforms();

		auto mainCameraView = world->mRegistry.view<FCamera>();
		if (mainCameraView.begin() == mainCameraView.end())
		{
			TURBO_LOG(LogSceneRendering, Error, "Scene doesn't contain any camera.")
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