#include "Layers/SceneRenderingLayer.h"

#include "Assets/AssetManager.h"
#include "Assets/StaticMesh.h"
#include "Core/CoreTimer.h"
#include "Core/Engine.h"
#include "Graphics/GeometryBuffer.h"
#include "Graphics/GPUDevice.h"
#include "Graphics/ResourceBuilders.h"
#include "Graphics/FrameGraph/RenderGraphUtils.h"
#include "Graphics/Shaders/SceneCullingCS.h"
#include "Graphics/Shaders/ToneMapperPostProcess.h"
#include "World/Camera.h"
#include "World/MeshComponent.h"
#include "World/ShadingComponents.h"
#include "World/World.h"

namespace Turbo
{
	struct FIndirectDrawBufferHeader
	{
		uint32 mNumDrawCalls = 0;
		uint32 __PADDING[3];
	};

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
		FGPUDevice& gpu = entt::locator<FGPUDevice>::value();
		mFrustumCullingPipeline = SceneCullingCS::CreatePipeline(gpu);
		mToneMapperPipeline = ToneMapperPostProcess::CreatePipeline(gpu);
	}

	void FSceneRenderingLayer::Shutdown()
	{
		FGPUDevice& gpu = entt::locator<FGPUDevice>::value();
		gpu.DestroyPipeline(mFrustumCullingPipeline);
		gpu.DestroyPipeline(mToneMapperPipeline);
	}

	FName FSceneRenderingLayer::GetName()
	{
		return GetStaticLayerName<FSceneRenderingLayer>();
	}

	void FSceneRenderingLayer::UpdateViewData(FWorld* world, FViewData& viewData)
	{
		TRACE_ZONE_SCOPED()

		auto mainCameraView = world->mRegistry.view<FCameraCache const, FWorldTransform const, FMainViewport const>();
		TURBO_CHECK(mainCameraView.begin() != mainCameraView.end())

		const entt::entity mainCameraEntity = *mainCameraView.begin();
		const FCameraCache& cameraCache = mainCameraView.get<FCameraCache>(mainCameraEntity);
		const FWorldTransform& cameraTransform = mainCameraView.get<FWorldTransform>(mainCameraEntity);

		viewData.mProjectionMatrix = cameraCache.mProjectionMatrix;
		viewData.mViewMatrix = glm::inverse(cameraTransform.mTransform);
		viewData.mWorldToProjection = viewData.mProjectionMatrix * viewData.mViewMatrix;
		viewData.mCameraPosition = TransformUtils::GetPosition(cameraTransform);

		viewData.mTime = FCoreTimer::TimeFromEngineStart();
		viewData.mWorldTime = FCoreTimer::TimeFromEngineStart();
		viewData.mDeltaTime = FCoreTimer::DeltaTime();

		FGPUDevice& gpu = entt::locator<FGPUDevice>::value();
		viewData.mFrameIndex = static_cast<int32>(gpu.GetNumRenderedFrames());

		viewData.mViewFrustum = cameraCache.mViewFrustum;
	}

	void FSceneRenderingLayer::CreateIndirectRenderBuffers(
		FRenderGraphBuilder& graphBuilder,
		FWorld* world,
		FSceneView* sceneView,
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

				const FDeviceSize indirectCommandsBufferSize = sizeof(FIndirectDrawBufferHeader) + numDraws * sizeof(vk::DrawIndirectCommand);
				const FRGBufferInfo indirectCommandsBufferInfo = {
					.mSize = indirectCommandsBufferSize,
					.mBufferFlags = EBufferFlags::CreateMapped | EBufferFlags::StorageBuffer | EBufferFlags::IndirectBuffer,
					.mName = FName(fmt::format("{}_IndirectCommands", material->mName))
				};
				drawIndirectBucket.mIndirectCommandBuffer = graphBuilder.CreateBuffer(indirectCommandsBufferInfo);

				FMaterial::IndirectDrawData* drawDatum = graphBuilder.AllocatePOD<FMaterial::IndirectDrawData>(numDraws);
				graphBuilder.QueueBufferUpload({
					.mTargetBuffer = drawIndirectBucket.mDrawBuffer,
					.mData = drawDatum,
					.mDataSize = numDraws * sizeof(FMaterial::IndirectDrawData),
				});

				const FViewData* viewData = sceneView->mViewData;

				// Fill buffers
				uint32 drawIndex = 0;
				for (FDrawCallIt drawCallIt = bucket.mStartIt; drawCallIt != bucket.mEndIt; ++drawCallIt)
				{
					FMaterial::IndirectDrawData& drawData = drawDatum[drawIndex];
					drawData.mModelToProj = viewData->mWorldToProjection * drawCallIt->mWorldTransform;
					drawData.mModelToView = viewData->mViewMatrix * drawCallIt->mWorldTransform;
					drawData.mModelToWorld = drawCallIt->mWorldTransform;
					drawData.mNormalModelToWorld = glm::float3x3(glm::transpose(glm::inverse(drawData.mModelToWorld)));

					drawData.mMaterialInstance = materialManager.GetMaterialInstanceAddress(gpu, drawCallIt->mMaterialInstance);
					drawData.mMaterialData = materialManager.GetMaterialDataAddress(gpu, bucket.mTargetMaterial);
					drawData.mMeshData = assetManager.GetMeshPointersAddress(gpu, drawCallIt->mMesh);

					drawIndex++;
				}
			}
		}
	}

	void FSceneRenderingLayer::Render(FRenderGraphBuilder& graphBuilder)
	{
		FWorld* world = gEngine->GetWorld();
		SceneGraph::UpdateWorldTransforms(world->mRegistry);
		FCameraUtils::UpdateDirtyCameras(world->mRegistry);
		FCameraUtils::UpdateCameraFrustum(world->mRegistry);
		SceneGraph::ClearDirtyFlags(world->mRegistry);

		auto mainCameraView = world->mRegistry.view<FCamera>();
		if (mainCameraView.begin() == mainCameraView.end())
		{
			TURBO_LOG(LogSceneRendering, Error, "Scene doesn't contain any camera.")
			return;
		}

		FSceneView* sceneView = graphBuilder.AllocatePOD<FSceneView>();

		RenderScene(graphBuilder, sceneView);
		RenderPostProcess(graphBuilder, sceneView);
	}

	void FSceneRenderingLayer::RenderScene(FRenderGraphBuilder& graphBuilder, FSceneView* sceneView)
	{
		TRACE_ZONE_SCOPED_N("Render Scene")

		FWorld* world = gEngine->GetWorld();

		// Create and upload view data
		sceneView->mViewData = graphBuilder.AllocatePOD<FViewData>();
		sceneView->mViewDataBufferHandle = graphBuilder.CreateBuffer({
			.mSize = sizeof(FViewData),
			.mBufferFlags = EBufferFlags::CreateMapped | EBufferFlags::UniformBuffer,
			.mName = FName("ViewDataBuffer")
		});

		UpdateViewData(world, *sceneView->mViewData);
		graphBuilder.QueueBufferUpload({
			.mTargetBuffer = sceneView->mViewDataBufferHandle,
			.mData = sceneView->mViewData,
			.mDataSize = sizeof(FViewData),
		});

		// Create Lights buffers
		std::vector<FLight> lights;
		auto pointLightView = world->mRegistry.view<FLightComponent, FWorldTransform>();
		for (const entt::entity entity : pointLightView)
		{
			const FWorldTransform& transform = pointLightView.get<FWorldTransform>(entity);
			const FLightComponent& pointLight = pointLightView.get<FLightComponent>(entity);

			if (pointLight.mIntensity > TURBO_SMALL_NUMBER)
			{
				lights.emplace_back(
					pointLight.mColor,
					pointLight.mIntensity,
					TransformUtils::GetPosition(transform),
					pointLight.mRange,
					TransformUtils::GetForward(transform),
					ForwardLightning::EncodeLightAnglesAndType(pointLight.mInnerAngle, pointLight.mOuterAngle, pointLight.mType)
				);
			}
		}

		if (lights.empty() == false)
		{
			std::tie(sceneView->mLightsBufferHandle, sceneView->mLights) =
				graphBuilder.CreateAndQueueBufferUpload<FLight>(FCreateAndUploadBuffer{
					.mData = lights.data(),
					.mSize = lights.size() * sizeof(FLight),
					.mBufferFlags = EBufferFlags::UniformBuffer,
					.mName = FName("PointLightBuffer")
				});
		}
		else
		{
			// This is a completely invalid solution. Let's do it!
			// TODO: Create dummy buffer in FEngineResources
			FLight dummyLight;

			std::tie(sceneView->mLightsBufferHandle, sceneView->mLights) =
				graphBuilder.CreateAndQueueBufferUpload<FLight>(FCreateAndUploadBuffer{
					.mData = &dummyLight,
					.mSize = sizeof(FLight),
					.mBufferFlags = EBufferFlags::UniformBuffer,
					.mName = FName("PointLightBuffer")
				});
		}

		// Create and upload scene data
		FSceneData* sceneData = graphBuilder.AllocatePOD<FSceneData>();
		sceneData->mNumLights = lights.size();
		std::tie(sceneView->mSceneDataBufferHandle, sceneView->mSceneData) =
			graphBuilder.CreateAndQueueBufferUpload<FSceneData>(FCreateAndUploadBuffer{
				.mData = sceneData,
				.mSize = sizeof(FSceneData),
				.mBufferFlags = EBufferFlags::UniformBuffer,
				.mName = FName("SceneDataBuffer")
			});

		std::vector<FDrawIndirectBucket> drawIndirectBuckets;
		CreateIndirectRenderBuffers(graphBuilder, world, sceneView, drawIndirectBuckets);

		// Fill IndirectCommandsBuffer header
		for (const FDrawIndirectBucket& bucket : drawIndirectBuckets)
		{
			RenderGraphUtils::AddFillBufferPass(graphBuilder, bucket.mIndirectCommandBuffer, 0, sizeof(FIndirectDrawBufferHeader), 0);
		}

		// Geometry culling
		static FName cullingPassName = FName("GeometryCullingPass");
		FRGPassInitializer cullingPass = graphBuilder.AddPass(cullingPassName, EPassType::Compute);

		cullingPass->ReadBuffer(sceneView->mViewDataBufferHandle);
		for (const FDrawIndirectBucket& bucket : drawIndirectBuckets)
		{
			cullingPass->ReadBuffer(bucket.mDrawBuffer);
			cullingPass->WriteBuffer(bucket.mIndirectCommandBuffer);
		}

		cullingPass->mExecutePass.BindLambda(
			[drawIndirectBuckets, pipeline = mFrustumCullingPipeline, sceneView](FGPUDevice& gpu, FCommandBuffer& cmd, FRenderResources& resources)
			{
				cmd.BindPipeline(pipeline);

				const FAssetManager& assetManager = entt::locator<FAssetManager>::value();
				const FBuffer* viewDataBuffer = gpu.AccessBuffer(resources.mBuffers.at(sceneView->mViewDataBufferHandle));

				SceneCullingCS::FPushConstants pushConstants = {
					.mViewData = viewDataBuffer->mDeviceAddress,
					.mBounds = assetManager.GetBoundsAddress(gpu)
				};

				for (const FDrawIndirectBucket& bucket : drawIndirectBuckets)
				{
					const FBuffer* drawBuffer = gpu.AccessBuffer(resources.mBuffers.at(bucket.mDrawBuffer));
					const FBuffer* indirectCommandBuffer = gpu.AccessBuffer(resources.mBuffers.at(bucket.mIndirectCommandBuffer));

					pushConstants.mDrawData = drawBuffer->mDeviceAddress;
					pushConstants.mDrawIndirectCommand = indirectCommandBuffer->mDeviceAddress;
					pushConstants.mNumDraws = bucket.mCount;

					cmd.PushConstants(pushConstants);

					const glm::uint3 groupCount = glm::uint3(Math::DivideAndRoundUp<uint32>(bucket.mCount, 64), 1, 1 );

					cmd.Dispatch(groupCount);
				}
			}
		);

		// Base Pass
		const static FName geometryPassName = FName("GeometryPass");
		FRGPassInitializer geometryPass = graphBuilder.AddPass(geometryPassName, EPassType::Graphics);

		FGeometryBuffer& geometryBuffer = entt::locator<FGeometryBuffer>::value();

		geometryPass->AddAttachment(
			{
				.mTexture = geometryBuffer.mSceneColor,
				.mLoadOp = ELoadOp::Clear,
				.mClearColor = EClearColor::OpaqueBlack
			},
			0);
		geometryPass->SetDepthStencilAttachment({
			.mTexture = geometryBuffer.mDepthStencil,
			.mLoadOp = ELoadOp::Clear,
			.mClearColor = EClearColor::Zero
		});

		geometryPass->ReadBuffer(sceneView->mViewDataBufferHandle);
		geometryPass->ReadBuffer(sceneView->mSceneDataBufferHandle);
		geometryPass->ReadBuffer(sceneView->mLightsBufferHandle);

		for (const FDrawIndirectBucket& bucket : drawIndirectBuckets)
		{
			geometryPass->ReadBuffer(bucket.mIndirectCommandBuffer);
			geometryPass->ReadBuffer(bucket.mDrawBuffer);
		}

		geometryPass->mExecutePass.BindLambda(
			[=](FGPUDevice& gpu, FCommandBuffer& cmd, FRenderResources& resources)
			{
				FMaterialManager& materialManager = entt::locator<FMaterialManager>::value();

				for (const FDrawIndirectBucket& bucket : drawIndirectBuckets)
				{
					TRACE_ZONE_SCOPED_N("Render Bucket")
					TRACE_GPU_SCOPED(gpu, cmd, "Render Bucket")

					const FMaterial* material = materialManager.AccessMaterial(bucket.mMaterialHandle);
					cmd.BindPipeline(material->mPipeline);
					cmd.BindDescriptorSet(gpu.GetBindlessResourcesSet(), 0);

					const FBuffer* drawBuffer = gpu.AccessBuffer(resources.mBuffers.at(bucket.mDrawBuffer));
					const FBuffer* viewDataBuffer = gpu.AccessBuffer(resources.mBuffers.at(sceneView->mViewDataBufferHandle));
					const FBuffer* sceneDataBuffer = gpu.AccessBuffer(resources.mBuffers.at(sceneView->mSceneDataBufferHandle));
					const FBuffer* lightsBuffer = gpu.AccessBuffer(resources.mBuffers.at(sceneView->mLightsBufferHandle));

					const FMaterial::PushConstants pushConstants = {
						.mViewData = viewDataBuffer->mDeviceAddress,
						.mSceneData = sceneDataBuffer->mDeviceAddress,
						.mLightData = lightsBuffer->mDeviceAddress,
						.mDrawData = drawBuffer->mDeviceAddress
					};

					THandle<FBuffer> commandBufferHandle = resources.mBuffers.at(bucket.mIndirectCommandBuffer);

					cmd.PushConstants(pushConstants);
					cmd.DrawIndirectCount(FDrawIndirectCountParams{
						.mBuffer = commandBufferHandle,
						.mOffset = sizeof(FIndirectDrawBufferHeader),
						.mCountBuffer = commandBufferHandle,
						.mCountOffset = offsetof(FIndirectDrawBufferHeader, mNumDrawCalls),
						.mMaxDrawCount = bucket.mCount,
						.mStride = sizeof(vk::DrawIndirectCommand),
					});
				}

				static const cstring kRenderBuckets = "Render Buckets";
				TRACE_PLOT_CONFIGURE(kRenderBuckets, EPlotFormat::Number, true, true, 0xFFFF00)
				TRACE_PLOT(kRenderBuckets, static_cast<int64>(drawIndirectBuckets.size()))
			}
		);
	}

	void FSceneRenderingLayer::RenderPostProcess(FRenderGraphBuilder& graphBuilder, FSceneView* SceneView)
	{
		TRACE_ZONE_SCOPED_N("Render Post-Process")

		const FGeometryBuffer& geometryBuffer = entt::locator<FGeometryBuffer>::value();

		// Tone Mapping
		{
			FWorld* world = gEngine->GetWorld();
			ToneMapperPostProcess::FComponent settings = {};

			if (const auto settingsView = world->mRegistry.view<ToneMapperPostProcess::FComponent>();
				settingsView.begin() != settingsView.end())
			{
				settings = settingsView.get<ToneMapperPostProcess::FComponent>(*settingsView.begin());
			}

			ToneMapperPostProcess::FUniformBuffer* uniformBufferData = graphBuilder.AllocatePOD<ToneMapperPostProcess::FUniformBuffer>();
			uniformBufferData->mExposure = 1.f / glm::pow(2.f, settings.mExposure);
			uniformBufferData->mSaturation = settings.mSaturation;
			uniformBufferData->mOffset = settings.mOffset;
			uniformBufferData->mSlope = settings.mSlope;
			uniformBufferData->mPower = settings.mPower;

			const static FName uniformBufferName = FName("ToneMapper.UniformBuffer");
			FRGResourceHandle uniformBuffer;
			std::tie(uniformBuffer, uniformBufferData) =
				graphBuilder.CreateAndQueueBufferUpload<ToneMapperPostProcess::FUniformBuffer>(FCreateAndUploadBuffer{
					.mData = uniformBufferData,
					.mSize = sizeof(ToneMapperPostProcess::FUniformBuffer),
					.mBufferFlags = EBufferFlags::UniformBuffer,
					.mName = uniformBufferName
				});

			const static FName passName = FName("ToneMapping");
			FRGPassInitializer pass = graphBuilder.AddPass(passName, EPassType::Compute);
			pass->ReadBuffer(uniformBuffer);
			pass->ReadTexture(geometryBuffer.mSceneColor);
			pass->WriteTexture(geometryBuffer.mAfterToneMap);

			pass->mExecutePass.BindLambda(
				[=, pipeline = mToneMapperPipeline](FGPUDevice& gpu, FCommandBuffer& cmd, FRenderResources& resources)
				{
					const THandle<FTexture> sceneColorHandle = resources.mTextures.at(geometryBuffer.mSceneColor);
					const FTextureCold* sceneColorCold = gpu.AccessTextureCold(sceneColorHandle);
					const THandle<FTexture> afterToneMapHandle = resources.mTextures.at(geometryBuffer.mAfterToneMap);
					const FBuffer* buffer = gpu.AccessBuffer(resources.mBuffers.at(uniformBuffer));

					ToneMapperPostProcess::FPushConstants pushConstants = {
						.mSceneColor = sceneColorHandle.GetIndex(),
						.mOutput = afterToneMapHandle.GetIndex(),
						.mTextureSize = sceneColorCold->GetSize2D(),
						.mUniforms = buffer->mDeviceAddress
					};

					cmd.BindPipeline(pipeline);
					cmd.PushConstants(pushConstants);
					cmd.BindDescriptorSet(gpu.GetBindlessResourcesSet(), 0);

					const glm::uint3 groupCount = Math::DivideAndRoundUp<glm::uint3>(
						sceneColorCold->GetSize(),
						glm::uint3(8, 8, 1)
					);
					cmd.Dispatch(groupCount);
				});
		}
	}

	bool FSceneRenderingLayer::ShouldRender()
	{
		return true;
	}
} // Turbo