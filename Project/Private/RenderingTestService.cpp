#include "imgui.h"
#include "RenderingTestLayer.h"
#include "Assets/AssetManager.h"

#include "Core/CoreTimer.h"
#include "Core/Engine.h"
#include "Core/Math/Vector.h"
#include "glm/gtx/compatibility.hpp"
#include "Graphics/GeometryBuffer.h"
#include "Graphics/GPUDevice.h"
#include "Graphics/ResourceBuilders.h"
#include "World/World.h"

using namespace Turbo;

struct FPushConstants
{
	glm::float4x4 objToProj;

	DeviceAddress positionBuffer;
	DeviceAddress normalBuffer;
	DeviceAddress uvBuffer;
	uint32 textureId;
	uint32 samplerId;
};

struct FRotateComponent
{
	float speed;
};

struct FCelestialBodyComponent
{
	glm::float3 color = ELinearColor::kWhite;
};

entt::entity CreatePivot(FWorld& world, entt::entity parent, float offset, float rotationSpeed)
{
	entt::registry& registry = world.mRegistry;

	// create entity
	const entt::entity entity = registry.create();
	registry.emplace<FRelationship>(entity);
	if (parent != entt::null)
	{
		world.AddChild(parent, entity);
	}

	// initialize components
	FTransform& transform = registry.emplace<FTransform>(entity);
	transform.mPosition.x = offset;
	FRotateComponent& rotateComponent = registry.emplace<FRotateComponent>(entity);
	rotateComponent.speed = rotationSpeed;

	return entity;
}

entt::entity CreateCelestialBody(FWorld& world, entt::entity parent, float offset, float rotationSpeed, glm::float3 color = ELinearColor::kWhite)
{
	entt::entity entity = CreatePivot(world, parent, offset, rotationSpeed);

	FCelestialBodyComponent& celestialBody = world.mRegistry.emplace<FCelestialBodyComponent>(entity);
	celestialBody.color = color;

    return entity;
}

FRenderingTestLayer::~FRenderingTestLayer()
{
}

FRenderingTestLayer::FRenderingTestLayer()
{
}

void FRenderingTestLayer::Start()
{
	TRACE_ZONE_SCOPED()

	FGPUDevice& gpu = entt::locator<FGPUDevice>::value();

	mMeshHandle = entt::locator<FAssetManager>::value().LoadMesh("Content/Meshes/SM_Cube.glb");
	mCatTexture = entt::locator<FAssetManager>::value().LoadTexture("Content/Textures/T_IndianaCat.dds");

	FSamplerBuilder samplerBuilder = {};
	samplerBuilder
		.SetMinMagFilter(vk::Filter::eNearest, vk::Filter::eLinear)
		.SetName(FName("CatSampler"));

	mCatSampler = gpu.CreateSampler(samplerBuilder);

	FDescriptorSetLayoutBuilder descriptorSetBuilder;
	descriptorSetBuilder
		.AddBinding(vk::DescriptorType::eSampler, 0, 1, {}, FName("Sampler"))
		.SetName(FName("GraphicsTest"));


	FPipelineBuilder graphicsPipelineBuilder;
	graphicsPipelineBuilder.SetPushConstantType<FPushConstants>();
	graphicsPipelineBuilder.SetName(FName("TestGraphicsPipeline"));

	graphicsPipelineBuilder.GetShaderState()
		.AddStage("PlanetShader", vk::ShaderStageFlagBits::eVertex)
		.AddStage("PlanetShader", vk::ShaderStageFlagBits::eFragment);

	graphicsPipelineBuilder.GetBlendState()
		.AddNoBlendingState();

	graphicsPipelineBuilder.GetDepthStencil()
		.SetDepth(true, true, vk::CompareOp::eGreaterOrEqual);

	graphicsPipelineBuilder.GetPipelineRendering()
		.AddColorAttachment(FGeometryBuffer::kColorFormat)
		.SetDepthAttachment(FGeometryBuffer::kDepthFormat);

	mGraphicsPipeline = gpu.CreatePipeline(graphicsPipelineBuilder);

	FWorld& world = *gEngine->GetWorld();

	const entt::entity sun = CreateCelestialBody(world, entt::null, 0.f, 0.f, ELinearColor::kYellow);

	for (uint32 planetId = 1; planetId < 3; ++planetId)
	{
		constexpr float offset = 3.f;
		const entt::entity pivot = CreatePivot(world, sun, 0.f, 1.f / static_cast<float>(planetId));
		const entt::entity planet = CreateCelestialBody(world, pivot, offset * static_cast<float>(planetId), 1.f, ELinearColor::kWhite);

		TURBO_LOG(LOG_TEMP, Info, "{} -> {} -> {}", (int32) sun, (int32) pivot, (int32) planet);
	}
}

void FRenderingTestLayer::Shutdown()
{
	FGPUDevice& gpu = entt::locator<FGPUDevice>::value();
	gpu.DestroyPipeline(mGraphicsPipeline);

	entt::locator<FAssetManager>::value().UnloadMesh(mMeshHandle);
	gpu.DestroyTexture(mCatTexture);
	gpu.DestroySampler(mCatSampler);
}

void FRenderingTestLayer::ShowImGuiWindow()
{
	ImGui::Begin("Rendering test");
	ImGui::Text("Frame time: %f, FPS: %f", FCoreTimer::DeltaTime(), 1.f / FCoreTimer::DeltaTime());
	ImGui::Separator();
	ImGui::DragFloat3("Camera Location", glm::value_ptr(mCameraLocation), 0.01f);
	ImGui::DragFloat2("Camera Rotation", glm::value_ptr(mCameraRotation), 1.f);
	ImGui::DragFloat("Camera FoV", &mCameraFov, 0.5f, 1.f);
	ImGui::DragFloat2("Camera Near Far", glm::value_ptr(mCameraNearFar), 0.5f );
	ImGui::End();
}

void FRenderingTestLayer::BeginTick(double deltaTime)
{
	FWorld& world = *gEngine->GetWorld();
	entt::registry& registry = world.mRegistry;
	auto rotationView = registry.view<FTransform, FRotateComponent>();

	for (const entt::entity entity : rotationView)
	{
		FTransform newTransform = rotationView.get<FTransform>(entity);
		const FRotateComponent& rotateComponent = rotationView.get<FRotateComponent>(entity);

		newTransform.mRotation = glm::quat(EVec3::Up * rotateComponent.speed * static_cast<float>(deltaTime)) * newTransform.mRotation;

		registry.replace<FTransform>(entity, newTransform);
	}

	ShowImGuiWindow();
}

void FRenderingTestLayer::PostBeginFrame(FGPUDevice* gpu, FCommandBuffer* cmd)
{
	TRACE_ZONE_SCOPED()
	TRACE_GPU_SCOPED(gpu, cmd, "RenderingTestLayer");

	const FGeometryBuffer& geometryBuffer = entt::locator<FGeometryBuffer>::value();

	FAssetManager& assetManager = entt::locator<FAssetManager>::value();
	const FMesh* mesh = assetManager.AccessMesh(mMeshHandle);
	const FSubMesh* subMesh = assetManager.AccessSubMesh(mesh->mSubMeshes[0]);

	const FBuffer* positionBuffer = gpu->AccessBuffer(subMesh->mPositionBuffer);
	const FBuffer* normalBuffer = gpu->AccessBuffer(subMesh->mNormalBuffer);
	const FBuffer* uvBuffer = gpu->AccessBuffer(subMesh->mUVBuffer);

	// Graphics pipeline
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

	cmd->BindPipeline(mGraphicsPipeline);
	cmd->BindIndexBuffer(subMesh->mIndicesBuffer);
	cmd->BindDescriptorSet(gpu->GetBindlessResourcesSet(), 0);

	glm::mat4 viewMat =
		glm::rotate(glm::mat4(1.f), glm::radians(mCameraRotation.x), EVec3::Right)
		* glm::rotate(glm::mat4(1.f), glm::radians(mCameraRotation.y), EVec3::Up)
		* glm::translate(glm::mat4(1.f), -mCameraLocation);

	const glm::vec2 resolution = geometryBuffer.GetResolution();
	glm::mat4 projMat = glm::perspectiveFov(glm::radians(mCameraFov), resolution.x, resolution.y, mCameraNearFar.y, mCameraNearFar.x);
	FMath::ConvertTurboToVulkanCoordinates(projMat);

	FPushConstants pushConstants = {};
	pushConstants.positionBuffer = positionBuffer->GetDeviceAddress();
	pushConstants.normalBuffer = normalBuffer->GetDeviceAddress();
	pushConstants.uvBuffer = uvBuffer->GetDeviceAddress();
	pushConstants.textureId = mCatTexture.mIndex;
	pushConstants.samplerId = mCatSampler.mIndex;

	FWorld& world = *gEngine->GetWorld();
	world.UpdateWorldTransforms();

	auto celestialBodiesView = world.mRegistry.view<FWorldTransform, FCelestialBodyComponent>();

	for (const entt::entity& celestialBody : celestialBodiesView)
	{
		const glm::float4x4 modelMat = celestialBodiesView.get<FWorldTransform>(celestialBody);
		const glm::float3 planetColor = celestialBodiesView.get<FCelestialBodyComponent>(celestialBody).color;

		pushConstants.objToProj = projMat * viewMat * modelMat;

		cmd->PushConstants(pushConstants);
		cmd->DrawIndexed(subMesh->mVertexCount);
	}

	cmd->EndRendering();
}

FName FRenderingTestLayer::GetName()
{
	const static FName kName = FName("RenderingTest");
	return kName;
}
