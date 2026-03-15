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
	}

	void FSceneRenderingLayer::Shutdown()
	{
	}

	void FSceneRenderingLayer::UpdateViewData(FWorld* world, FViewData& viewData)
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

		FGPUDevice& gpu = entt::locator<FGPUDevice>::value();
		viewData.mFrameIndex = static_cast<int32>(gpu.GetNumRenderedFrames());
	}

	void FSceneRenderingLayer::CreateIndirectRenderBuffers(
		FRenderGraphBuilder& graphBuilder,
		FWorld* world,
		FViewData& viewData,
		std::vector<FDrawIndirectBucket>& outBuckets
	)
	{
		TRACE_ZONE_SCOPED()

		entt::registry& registry = world->mRegistry;

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
			TRACE_ZONE_SCOPED_N("Initialize render buckets' buffers")
			const FMaterialManager& materialManager = entt::locator<FMaterialManager>::value();
			const FAssetManager& assetManager = entt::locator<FAssetManager>::value();
			const FGPUDevice& gpu = entt::locator<FGPUDevice>::value();

			uint32 numBuckets = materialBuckets.size();
			outBuckets.reserve(numBuckets);

			for (const FMaterialBucket& bucket : materialBuckets)
			{
				FDrawIndirectBucket& drawIndirectBucket = outBuckets.emplace_back();
				drawIndirectBucket.mMaterialHandle = bucket.mTargetMaterial;

				const FMaterial* material = materialManager.AccessMaterial(bucket.mTargetMaterial);
				const uint32 numDraws = bucket.mEndIt - bucket.mStartIt;
				drawIndirectBucket.mCount = numDraws;

				// Initialize buffers
				const FRGBufferInfo drawDataBufferInfo = {
					.mSize = numDraws * sizeof(FMaterial::IndirectDrawData),
					.mBufferFlags = EBufferFlags::CreateMapped | EBufferFlags::StorageBuffer,
					.mName = FName(fmt::format("{}_DrawData", material->mName))
				};
				drawIndirectBucket.mDrawBuffer = graphBuilder.CreateBuffer(drawDataBufferInfo);

				const FRGBufferInfo indirectCommandsBufferInfo = {
					.mSize = numDraws * sizeof(vk::DrawIndirectCommand),
					.mBufferFlags = EBufferFlags::CreateMapped | EBufferFlags::StorageBuffer | EBufferFlags::IndirectBuffer,
					.mName = FName(fmt::format("{}_IndirectCommands", material->mName))
				};
				drawIndirectBucket.mIndirectCommandBuffer = graphBuilder.CreateBuffer(indirectCommandsBufferInfo);

				// Allocate cpu data and queue upload to target buffers
				FMaterial::IndirectDrawData* drawDatum = graphBuilder.AllocatePOD<FMaterial::IndirectDrawData>(numDraws);
				graphBuilder.QueueBufferUpload({
					.mTargetBuffer = drawIndirectBucket.mDrawBuffer,
					.mData = drawDatum,
					.mDataSize = numDraws * sizeof(FMaterial::IndirectDrawData),
				});
				vk::DrawIndirectCommand* drawCommands = graphBuilder.AllocatePOD<vk::DrawIndirectCommand>(numDraws);
				graphBuilder.QueueBufferUpload({
					.mTargetBuffer = drawIndirectBucket.mIndirectCommandBuffer,
					.mData = drawCommands,
					.mDataSize = numDraws * sizeof(vk::DrawIndirectCommand),
				});

				// Fill buffers
				uint32 drawIndex = 0;
				for (FDrawCallIt drawCallIt = bucket.mStartIt; drawCallIt != bucket.mEndIt; ++drawCallIt)
				{
					const FMesh* mesh = assetManager.AccessMesh(drawCallIt->mMesh);

					FMaterial::IndirectDrawData& drawData = drawDatum[drawIndex];
					drawData.mModelToProj = viewData.mWorldToProjection * drawCallIt->mWorldTransform;
					drawData.mModelToView = viewData.mViewMatrix * drawCallIt->mWorldTransform;
					drawData.mNormalModelToView = glm::float3x3(glm::transpose(glm::inverse(drawData.mModelToView)));

					drawData.mMaterialInstance = materialManager.GetMaterialInstanceAddress(gpu, drawCallIt->mMaterialInstance);
					drawData.mMaterialData = materialManager.GetMaterialDataAddress(gpu, bucket.mTargetMaterial);
					drawData.mMeshData = assetManager.GetMeshPointersAddress(gpu, drawCallIt->mMesh);

					vk::DrawIndirectCommand& command = drawCommands[drawIndex];
					command.vertexCount = mesh->mVertexCount;
					command.firstInstance = drawIndex;
					command.instanceCount = 1;
					command.firstVertex = 1;

					drawIndex++;
				}
			}
		}
	}

	void FSceneRenderingLayer::DrawIndirect(
		FGPUDevice& gpu,
		FCommandBuffer& cmd,
		FRenderResources& resources,
		FRGResourceHandle viewDataBufferHandle,
		const std::vector<FDrawIndirectBucket>& buckets
	)
	{
		TRACE_ZONE_SCOPED_N("Render Basepass")
		TRACE_GPU_SCOPED(gpu, cmd, "Render Basepass")

		FMaterialManager& materialManager = entt::locator<FMaterialManager>::value();

		for (int32 bucketId = 0; bucketId < buckets.size(); ++bucketId)
		{
			TRACE_ZONE_SCOPED_N("Render Bucket")
			TRACE_GPU_SCOPED(gpu, cmd, "Render Bucket")

			const FDrawIndirectBucket& bucket = buckets[bucketId];

			const FMaterial* material = materialManager.AccessMaterial(bucket.mMaterialHandle);
			cmd.BindPipeline(material->mPipeline);
			cmd.BindDescriptorSet(gpu.GetBindlessResourcesSet(), 0);

			const FBuffer* drawBuffer = gpu.AccessBuffer(resources.mBuffers.at(bucket.mDrawBuffer));
			const FBuffer* viewDataBuffer = gpu.AccessBuffer(resources.mBuffers.at(viewDataBufferHandle));

			const FMaterial::PushConstants pushConstants = {
				.mViewData = viewDataBuffer->mDeviceAddress,
				.mDrawData = drawBuffer->mDeviceAddress
			};

			cmd.PushConstants(pushConstants);
			cmd.DrawIndirect(
				resources.mBuffers.at(bucket.mIndirectCommandBuffer),
				0,
				bucket.mCount,
				sizeof(vk::DrawIndirectCommand)
			);
		}

		static const cstring kRenderBuckets = "Render Buckets";
		TRACE_PLOT_CONFIGURE(kRenderBuckets, EPlotFormat::Number, true, true, 0xFFFF00)
		TRACE_PLOT(kRenderBuckets, static_cast<int64>(buckets.size()))
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

		FViewData* viewData = graphBuilder.AllocatePOD<FViewData>();
		FRGResourceHandle viewDataBuffer = graphBuilder.CreateBuffer({
			.mSize = sizeof(FViewData),
			.mBufferFlags = EBufferFlags::CreateMapped | EBufferFlags::UniformBuffer,
			.mName = FName("ViewDataBuffer")
		});

		UpdateViewData(world, *viewData);
		graphBuilder.QueueBufferUpload({
			.mTargetBuffer = viewDataBuffer,
			.mData = viewData,
			.mDataSize = sizeof(FViewData),
		});

		std::vector<FDrawIndirectBucket> drawIndirectBuckets;
		CreateIndirectRenderBuffers(graphBuilder, world, *viewData, drawIndirectBuckets);

		graphBuilder.AddPass(
			FName("GeometryPass"),
			FRGSetupPassDelegate::CreateLambda(
				[&](FRGPassInfo& passInfo)
				{
					passInfo.mPassType = EPassType::Graphics;

					FGeometryBuffer& geometryBuffer = entt::locator<FGeometryBuffer>::value();
					passInfo.AddAttachment(geometryBuffer.mColor, 0);
					passInfo.SetDepthStencilAttachment(geometryBuffer.mDepth);

					passInfo.ReadBuffer(viewDataBuffer);

					for (const FDrawIndirectBucket& bucket : drawIndirectBuckets)
					{
						passInfo.ReadBuffer(bucket.mIndirectCommandBuffer);
						passInfo.ReadBuffer(bucket.mDrawBuffer);
					}
				}),
			FRGExecutePassDelegate::CreateLambda(
				[viewDataBuffer, drawIndirectBuckets](FGPUDevice& gpu, FCommandBuffer& cmd, FRenderResources& resources)
				{
					TRACE_ZONE_SCOPED_N("Render Basepass")
					TRACE_GPU_SCOPED(gpu, cmd, "Render Basepass")

					DrawIndirect(gpu, cmd, resources, viewDataBuffer, drawIndirectBuckets);
				})
		);
	}

	bool FSceneRenderingLayer::ShouldRender()
	{
		return true;
	}

} // Turbo