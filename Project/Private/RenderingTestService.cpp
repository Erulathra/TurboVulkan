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

	vk::DeviceAddress positionBuffer;
	vk::DeviceAddress normalBuffer;
	vk::DeviceAddress colorBuffer;
	uint64 _PADDING;

	glm::float4 color = ELinearColor::kYellow;
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

	mMeshHandle = entt::locator<FAssetManager>::value().LoadMesh("Content/Meshes/IcoPlanet.glb");

	FDescriptorSetLayoutBuilder descriptorSetBuilder;
	descriptorSetBuilder
		.AddBinding(vk::DescriptorType::eStorageBuffer, 0, FName("positions"))
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

	FGPUDevice& gpu = entt::locator<FGPUDevice>::value();
	mGraphicsPipeline = gpu.CreatePipeline(graphicsPipelineBuilder);

	FWorld& world = *gEngine->GetWorld();

	const entt::entity sun = CreateCelestialBody(world, entt::null, 0.f, 0.f, ELinearColor::kYellow);

	for (uint32 planetId = 1; planetId < 3; ++planetId)
	{
		constexpr float offset = 4.f;
		const entt::entity pivot = CreatePivot(world, sun, 0.f, 45.f / static_cast<float>(planetId));
		CreateCelestialBody(world, pivot, offset * static_cast<float>(planetId), 90.f, ELinearColor::kWhite);
	}
}

void FRenderingTestLayer::Shutdown()
{
	FGPUDevice& gpu = entt::locator<FGPUDevice>::value();
	gpu.DestroyPipeline(mGraphicsPipeline);

	entt::locator<FAssetManager>::value().UnloadMesh(mMeshHandle);
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

void FRenderingTestLayer::BeginTick_GameThread(double deltaTime)
{
	FWorld& world = *gEngine->GetWorld();
	entt::registry& registry = world.mRegistry;
	auto rotationView = registry.view<FTransform, FRotateComponent>();

	for (const entt::entity entity : rotationView)
	{
		FTransform newTransform = rotationView.get<FTransform>(entity);
		const FRotateComponent& rotateComponent = rotationView.get<FRotateComponent>(entity);

		newTransform.mRotation = glm::quat(EVec3::Up * glm::radians(rotateComponent.speed)) * newTransform.mRotation;

		registry.replace<FTransform>(entity, newTransform);
	}

	ShowImGuiWindow();
}

void FRenderingTestLayer::PostBeginFrame_RenderThread(FGPUDevice* gpu, FCommandBuffer* cmd)
{
	TRACE_ZONE_SCOPED()
	TRACE_GPU_SCOPED(gpu, cmd, "RenderingTestLayer");

	const FGeometryBuffer& geometryBuffer = entt::locator<FGeometryBuffer>::value();

	const FSubMesh* mesh = entt::locator<FAssetManager>::value().AccessMesh(mMeshHandle);
	const FBuffer* positionBuffer = gpu->AccessBuffer(mesh->mPositionBuffer);
	const FBuffer* normalBuffer = gpu->AccessBuffer(mesh->mNormalBuffer);
	const FBuffer* colorBuffer = gpu->AccessBuffer(mesh->mColorBuffer);

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
	cmd->BindIndexBuffer(mesh->mIndicesBuffer);

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
	pushConstants.colorBuffer = colorBuffer->GetDeviceAddress();

	FWorld& world = *gEngine->GetWorld();
	world.UpdateWorldTransforms();

	auto celestialBodiesView = world.mRegistry.view<FWorldTransform, FCelestialBodyComponent>();
	for (const entt::entity& celestialBody : celestialBodiesView)
	{
		const glm::float4x4 modelMat = celestialBodiesView.get<FWorldTransform>(celestialBody);
		const glm::float3 planetColor = celestialBodiesView.get<FCelestialBodyComponent>(celestialBody).color;

		pushConstants.objToProj = projMat * viewMat * modelMat;
		pushConstants.color = glm::float4(planetColor, 1.f);

		cmd->PushConstants(pushConstants);
		cmd->DrawIndexed(mesh->mVertexCount);
	}

	cmd->EndRendering();
}

FName FRenderingTestLayer::GetName()
{
	const static FName kName = FName("RenderingTest");
	return kName;
}
