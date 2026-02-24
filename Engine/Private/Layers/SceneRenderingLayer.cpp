#include "Layers/SceneRenderingLayer.h"

#include "Assets/AssetManager.h"
#include "Assets/EngineResources.h"
#include "Assets/StaticMesh.h"
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
	struct FDrawCall
	{
		glm::float4x4 mWorldTransform = glm::float4x4(1.f);

		THandle<FMesh> mMesh = {};
		THandle<FMaterial> mMaterial = {};
		THandle<FMaterial::Instance> mMaterialInstance = {};

		uint64 mDrawCallHash = std::numeric_limits<uint64>::max();

		glm::float3 mBoundsMin = {};
		glm::float3 mBoundsMax = {};
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

	void FSceneRenderingLayer::Shutdown()
	{
		FGPUDevice& gpu = entt::locator<FGPUDevice>::value();
		gpu.DestroyBuffer(mViewDataUniformBuffer);
	}

	void FSceneRenderingLayer::UpdateViewData(FGPUDevice& gpu, FCommandBuffer& cmd, FWorld* world, FViewData& viewData)
	{
		TRACE_ZONE_SCOPED()

		FCameraUtils::UpdateDirtyCameras(world->mRegistry);

		auto mainCameraView = world->mRegistry.view<FCameraCache const, FWorldTransform const, FMainViewport const>();
		TURBO_CHECK(mainCameraView.begin() != mainCameraView.end())

		const entt::entity mainCameraEntity = *mainCameraView.begin();
		const FCameraCache& cameraCache = mainCameraView.get<FCameraCache>(mainCameraEntity);
		const FWorldTransform& cameraTransform = mainCameraView.get<FWorldTransform>(mainCameraEntity);

		viewData.mProjectionMatrix = cameraCache.mProjectionMatrix;
		viewData.mViewMatrix = glm::inverse(cameraTransform.mTransform);
		viewData.mWorldToProjection = viewData.mProjectionMatrix * viewData.mViewMatrix;

		viewData.mTime = FCoreTimer::TimeFromEngineStart();
		viewData.mWorldTime = FCoreTimer::TimeFromEngineStart();
		viewData.mDeltaTime = FCoreTimer::DeltaTime();
		viewData.mFrameIndex = static_cast<int32>(gpu.GetNumRenderedFrames());

		const FBuffer* viewDataBuffer = gpu.AccessBuffer(mViewDataUniformBuffer);
		std::memcpy(viewDataBuffer->GetMappedAddress(), &viewData, sizeof(FViewData));

#if 0 // This barrier is unnecessary
		cmd.BufferBarrier(
			mViewDataUniformBuffer,
			vk::AccessFlagBits2::eHostWrite,
			vk::PipelineStageFlagBits2::eHost,
			vk::AccessFlagBits2::eShaderRead,
			vk::PipelineStageFlagBits2::eVertexShader
		);
#endif
	}

	void FSceneRenderingLayer::RenderMeshes(FGPUDevice& gpu, FCommandBuffer& cmd, FWorld* world, const FViewData& viewData)
	{
		TRACE_ZONE_SCOPED()

		entt::registry& registry = world->mRegistry;
		const uint32 frameId = gpu.GetBufferedFrameId();

		entt::storage<FDrawCall> drawCalls;
		using FDrawCallIt = entt::storage<FDrawCall>::iterator;

		struct FMaterialBucket
		{
			THandle<FMaterial> mTargetMaterial = {};
			FDrawCallIt mStartIt = {};
			FDrawCallIt mEndIt = {};
		};

		std::vector<FMaterialBucket> materialBuckets;

		{
			TRACE_ZONE_SCOPED_N("Prepare drawcalls")

			const auto meshView = registry.view<FMeshComponent>();

			{
				TRACE_ZONE_SCOPED_N("Reserve storage")
				drawCalls.reserve(meshView.storage()->size());

				for (entt::entity entity : meshView)
				{
					drawCalls.emplace(entity);
				}
			}

			{
				TRACE_ZONE_SCOPED_N("Calculate transforms")

				const auto meshTransformView = registry.view<FMeshComponent, FWorldTransform>();
				for (entt::entity entity : meshTransformView)
				{
					FWorldTransform& worldTransformComp = meshTransformView.get<FWorldTransform>(entity);
					drawCalls.get(entity).mWorldTransform = worldTransformComp.mTransform;
				}

				const auto meshRelationView = registry.view<FMeshComponent, FRelationship>(entt::exclude<FWorldTransform>);
				for (entt::entity entity : meshRelationView)
				{
					const FRelationship& relationship = meshRelationView.get<FRelationship>(entity);
					drawCalls.get(entity).mWorldTransform = registry.get<FWorldTransform>(relationship.mParent).mTransform;
				}
			}

			{
				TRACE_ZONE_SCOPED_N("Fill draw calls")
				for (entt::entity entity : meshView)
				{
					const FMeshComponent& meshComponent = meshView.get<FMeshComponent>(entity);

					FDrawCall& currentDrawCall = drawCalls.get(entity);
					currentDrawCall.mMesh = meshComponent.mMesh;
					currentDrawCall.mMaterial = meshComponent.mMaterial;
					currentDrawCall.mMaterialInstance = meshComponent.mMaterialInstance;

					constexpr uint64 kMaterialMask = 0xFFFF000000000000;
					constexpr uint64 kMaterialInstanceMask = 0x0000FFFF00000000;
					constexpr uint64 kMeshMask = 0x00000000FFFF0000;

					TURBO_CHECK(currentDrawCall.mMaterial.GetIndex() < 1 << std::popcount(kMaterialMask))
					TURBO_CHECK(currentDrawCall.mMaterial.GetIndex() < 1 << std::popcount(kMaterialInstanceMask))
					TURBO_CHECK(currentDrawCall.mMaterial.GetIndex() < 1 << std::popcount(kMeshMask))

					currentDrawCall.mDrawCallHash =
						static_cast<uint64>(currentDrawCall.mMaterial.GetIndex()) << std::countr_zero(kMaterialMask)
						| static_cast<uint64>(currentDrawCall.mMaterialInstance.GetIndex()) << std::countr_zero(kMaterialInstanceMask)
						| static_cast<uint64>(currentDrawCall.mMesh.GetIndex()) << std::countr_zero(kMeshMask);
				}
			}
		}

		{
			TRACE_ZONE_SCOPED_N("Sort draw calls")
			drawCalls.sort([&](entt::entity left, const entt::entity& right)
			{
				return drawCalls.get(left).mDrawCallHash < drawCalls.get(right).mDrawCallHash;
			});
		}

		{
			materialBuckets.emplace_back();
			FMaterialBucket& currentBucket = materialBuckets.front();
			currentBucket.mTargetMaterial = drawCalls.begin()->mMaterial;
			currentBucket.mStartIt = drawCalls.begin();

			TRACE_ZONE_SCOPED_N("Create material buckets")
			for (FDrawCallIt drawCallIt = drawCalls.begin(); drawCallIt != drawCalls.end(); ++drawCallIt)
			{
				if (drawCallIt->mMaterial != currentBucket.mTargetMaterial)
				{
					currentBucket.mEndIt = drawCallIt;

					materialBuckets.emplace_back();
					currentBucket = materialBuckets.front();
					currentBucket.mTargetMaterial = drawCallIt->mMaterial;
					currentBucket.mStartIt = drawCallIt;
				}
			}

			currentBucket.mEndIt = drawCalls.end();
		}

		{
			TRACE_ZONE_SCOPED_N("Render meshes")

			const FAssetManager& assetManager = entt::locator<FAssetManager>::value();
			const FMaterialManager& materialManager = entt::locator<FMaterialManager>::value();

			THandle<FMaterial::Instance> materialInstanceHandle;
			THandle<FMesh> meshHandle = EngineResources::GetPlaceholderMesh();
			const FMesh* mesh = assetManager.AccessMesh(meshHandle);

			FDeviceAddress viewDataDeviceAddress = gpu.AccessBuffer(mViewDataUniformBuffer)->GetDeviceAddress();

#if WITH_PROFILER
			FCounterType numPipelineSwitches = 0;
#endif // WITH_PROFILER
			for (const FMaterialBucket& materialBucket : materialBuckets)
			{
				{
					TRACE_ZONE_SCOPED_N("Bind Material")

					const FMaterial* material = materialManager.AccessMaterial(materialBucket.mTargetMaterial);
					cmd.BindPipeline(material->mPipeline);
					cmd.BindDescriptorSet(gpu.GetBindlessResourcesSet(), 0);

#if WITH_PROFILER
					numPipelineSwitches++;
#endif // WITH_PROFILER
				}

				for (FDrawCallIt drawCallIt = materialBucket.mStartIt; drawCallIt != materialBucket.mEndIt; ++drawCallIt)
				{
					TRACE_ZONE_SCOPED_N("Render entity")
					TRACE_GPU_SCOPED(gpu, cmd, "Render entity");

					if (drawCallIt->mMaterialInstance != materialInstanceHandle)
					{
						materialInstanceHandle = drawCallIt->mMaterialInstance;
					}

					if (drawCallIt->mMesh != meshHandle)
					{
						TRACE_ZONE_SCOPED_N("Bind index buffer")

						meshHandle = drawCallIt->mMesh;
						mesh = assetManager.AccessMesh(meshHandle);

						cmd.BindIndexBuffer(mesh->mIndicesBuffer);
					}

					FMaterial::PushConstants pushConstants = {};
					pushConstants.mModelToProj = viewData.mWorldToProjection * drawCallIt->mWorldTransform;
					pushConstants.mModelToView = viewData.mViewMatrix * drawCallIt->mWorldTransform;
					pushConstants.mInvModelToView = glm::float3x3(glm::transpose(glm::inverse(pushConstants.mModelToView)));

					pushConstants.mViewData = viewDataDeviceAddress;
					pushConstants.mMaterialInstance = materialManager.GetMaterialInstanceAddress(gpu, materialInstanceHandle);
					pushConstants.mMaterialData = materialManager.GetMaterialDataAddress(gpu, materialBucket.mTargetMaterial);
					pushConstants.mMeshData = assetManager.GetMeshPointersAddress(gpu, meshHandle);
					pushConstants.mSceneData = kNullDeviceAddress;

					if (mesh != nullptr)
					{
						TRACE_ZONE_SCOPED_N("Draw Indexed")

						cmd.PushConstants(pushConstants);
						cmd.DrawIndexed(mesh->mVertexCount);
					}
				}
			}

			static const cstring kDrawCallPlotName = "Draw calls";
			TRACE_PLOT_CONFIGURE(kDrawCallPlotName, EPlotFormat::Number, true, true, 0xFF00FF)
			TRACE_PLOT(kDrawCallPlotName, static_cast<int64>(drawCalls.size()))

			static const cstring kPipelineSwitchesName = "Pipeline switches";
			TRACE_PLOT_CONFIGURE(kPipelineSwitchesName, EPlotFormat::Number, true, true, 0xFFFF00)
			TRACE_PLOT(kPipelineSwitchesName, numPipelineSwitches)
		}
	}

	void FSceneRenderingLayer::RenderScene(FRenderGraphBuilder& graphBuilder)
	{
		TRACE_ZONE_SCOPED_N("Render Scene")

		FWorld* world = gEngine->GetWorld();
		FSceneGraph::UpdateWorldTransforms(world->mRegistry);

		auto mainCameraView = world->mRegistry.view<FCamera>();
		if (mainCameraView.begin() == mainCameraView.end())
		{
			TURBO_LOG(LogSceneRendering, Error, "Scene doesn't contain any camera.")
			return;
		}

		graphBuilder.AddPass(
			FName("GeometryPass"),
			FRGSetupPassDelegate::CreateLambda(
				[&](FRGPassInfo& passInfo)
				{
					passInfo.mPassType = EPassType::Graphics;

					FGeometryBuffer& geometryBuffer = entt::locator<FGeometryBuffer>::value();
					passInfo.AddAttachment(geometryBuffer.mColor, 0);
					passInfo.SetDepthStencilAttachment(geometryBuffer.mDepth);
				}),
			FRGExecutePassDelegate::CreateLambda(
				// todo: Capturing this is dangerous, but before adding buffer support to render graph, It is necessary.
				[this](FGPUDevice& gpu, FCommandBuffer& cmd, FRenderResources& resources)
				{
					TRACE_ZONE_SCOPED_N("Rendering ImGui")
					TRACE_GPU_SCOPED(gpu, cmd, "Rendering ImGUI")

					FWorld* world = gEngine->GetWorld();

					FViewData viewData;
					UpdateViewData(gpu, cmd, world, viewData);
					RenderMeshes(gpu, cmd, world, viewData);
				})
		);
	}

	bool FSceneRenderingLayer::ShouldRender()
	{
		return true;
	}

} // Turbo