#include "imgui.h"
#include "RenderingTestLayer.h"
#include "Assets/AssetManager.h"

#include "Core/CoreTimer.h"
#include "Core/Engine.h"
#include "Core/Math/Vector.h"
#include "Graphics/GeometryBuffer.h"
#include "Graphics/GPUDevice.h"
#include "Graphics/ResourceBuilders.h"

using namespace Turbo;

struct FPushConstants
{
	glm::mat4 objToProj;
	vk::DeviceAddress positionBuffer;
	vk::DeviceAddress normalBuffer;
	vk::DeviceAddress colorBuffer;
};

FRenderingTestLayer::~FRenderingTestLayer()
{
}

FRenderingTestLayer::FRenderingTestLayer()
{
}

void FRenderingTestLayer::Start()
{
	TRACE_ZONE_SCOPED()

	mMeshHandle = entt::locator<FAssetManager>::value().LoadMesh("Content/Meshes/BlenderMonkey.glb");

	FDescriptorSetLayoutBuilder descriptorSetBuilder;
	descriptorSetBuilder
		.AddBinding(vk::DescriptorType::eStorageBuffer, 0, FName("positions"))
		.SetName(FName("GraphicsTest"));

	FPipelineBuilder graphicsPipelineBuilder;
	graphicsPipelineBuilder.SetPushConstantType<FPushConstants>();
	graphicsPipelineBuilder.SetName(FName("TestGraphicsPipeline"));

	graphicsPipelineBuilder.GetShaderState()
		.AddStage("GPShader", vk::ShaderStageFlagBits::eVertex)
		.AddStage("GPShader", vk::ShaderStageFlagBits::eFragment);

	graphicsPipelineBuilder.GetBlendState()
		.AddNoBlendingState();

	graphicsPipelineBuilder.GetDepthStencil()
		.SetDepth(true, true, vk::CompareOp::eGreaterOrEqual);

	graphicsPipelineBuilder.GetPipelineRendering()
		.AddColorAttachment(FGeometryBuffer::kColorFormat)
		.SetDepthAttachment(FGeometryBuffer::kDepthFormat);

	FGPUDevice& gpu = entt::locator<FGPUDevice>::value();
	mGraphicsPipeline = gpu.CreatePipeline(graphicsPipelineBuilder);
}

void FRenderingTestLayer::Shutdown()
{
	FGPUDevice& gpu = entt::locator<FGPUDevice>::value();
	gpu.DestroyPipeline(mGraphicsPipeline);

	entt::locator<FAssetManager>::value().UnloadMesh(mMeshHandle);
}

void FRenderingTestLayer::BeginTick_GameThread(double deltaTime)
{
	ImGui::Begin("Rendering test");
	ImGui::Text("Frame time: %f, FPS: %f", FCoreTimer::DeltaTime(), 1.f / FCoreTimer::DeltaTime());
	ImGui::Separator();
	ImGui::DragFloat3("Model Location", glm::value_ptr(mModelLocation), 0.01f);
	ImGui::Separator();
	ImGui::DragFloat3("Camera Location", glm::value_ptr(mCameraLocation), 0.01f);
	ImGui::DragFloat2("Camera Rotation", glm::value_ptr(mCameraRotation), 1.f);
	ImGui::DragFloat("Camera FoV", &mCameraFov, 0.5f, 1.f);
	ImGui::DragFloat2("Camera Near Far", glm::value_ptr(mCameraNearFar), 0.5f );
	ImGui::End();
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

	const glm::mat4 modelMat = glm::translate(glm::mat4(1.f), mModelLocation);

	glm::mat4 viewMat =
		glm::rotate(glm::mat4(1.f), glm::radians(mCameraRotation.x), EVec3::Right)
		* glm::rotate(glm::mat4(1.f), glm::radians(mCameraRotation.y), EVec3::Up)
		* glm::translate(glm::mat4(1.f), -mCameraLocation);

	const glm::vec2 resolution = geometryBuffer.GetResolution();
	glm::mat4 projMat = glm::perspectiveFov(glm::radians(mCameraFov), resolution.x, resolution.y, mCameraNearFar.y, mCameraNearFar.x);
	FMath::ConvertTurboToVulkanCoordinates(projMat);

	FPushConstants pushConstants = {};
	pushConstants.objToProj = projMat * viewMat * modelMat;
	pushConstants.positionBuffer = positionBuffer->GetDeviceAddress();
	pushConstants.normalBuffer = normalBuffer->GetDeviceAddress();
	pushConstants.colorBuffer = colorBuffer->GetDeviceAddress();

	cmd->BindPipeline(mGraphicsPipeline);
	cmd->PushConstants(pushConstants);
	cmd->BindIndexBuffer(mesh->mIndicesBuffer);
	cmd->DrawIndexed(mesh->mVertexCount);

	cmd->EndRendering();
}

FName FRenderingTestLayer::GetName()
{
	const static FName kName = FName("RenderingTest");
	return kName;
}