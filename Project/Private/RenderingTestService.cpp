#include "imgui.h"
#include "RenderingTestLayer.h"
#include "Assets/AssetManager.h"

#include "Core/CoreTimer.h"
#include "Core/Engine.h"
#include "Core/Math/Vector.h"
#include "Graphics/GeometryBuffer.h"
#include "Graphics/GPUDevice.h"
#include "Graphics/GraphicsLocator.h"
#include "Graphics/ResourceBuilders.h"

using namespace Turbo;

struct FPushConstants
{
	glm::mat4 objToProj;
	glm::mat4 objToView;
};

FRenderingTestLayer::~FRenderingTestLayer()
{
}

FRenderingTestLayer::FRenderingTestLayer()
{
}

void FRenderingTestLayer::Start()
{
	FGPUDevice* gpu = gEngine->GetGpu();

	mMeshHandle = gEngine->GetAssetManager()->LoadMesh("Content/Meshes/BlenderMonkey.glb");

	FDescriptorSetLayoutBuilder descriptorSetBuilder;
	descriptorSetBuilder
		.AddBinding(vk::DescriptorType::eStorageBuffer, 0, FName("positions"))
		.SetName(FName("GraphicsTest"));

	mGraphicsPipelineSetLayout = gpu->CreateDescriptorSetLayout(descriptorSetBuilder);

	FPipelineBuilder graphicsPipelineBuilder;
	graphicsPipelineBuilder.AddDescriptorSetLayout(mGraphicsPipelineSetLayout);
	graphicsPipelineBuilder.SetPushConstantType<FPushConstants>();
	graphicsPipelineBuilder.SetName(FName("TestGraphicsPipeline"));
	graphicsPipelineBuilder.GetRasterization().SetCullMode(vk::CullModeFlagBits::eNone);

	graphicsPipelineBuilder.GetShaderState()
		.AddStage("GPShader", vk::ShaderStageFlagBits::eVertex)
		.AddStage("GPShader", vk::ShaderStageFlagBits::eFragment);

	graphicsPipelineBuilder.GetBlendState()
		.AddNoBlendingState();

	graphicsPipelineBuilder.GetPipelineRendering()
		.AddColorAttachment(FGeometryBuffer::kColorFormat);

	mGraphicsPipeline = gpu->CreatePipeline(graphicsPipelineBuilder);
}

void FRenderingTestLayer::Shutdown()
{
	FGPUDevice* gpu = gEngine->GetGpu();

	gpu->DestroyPipeline(mGraphicsPipeline);
	gpu->DestroyDescriptorSetLayout(mGraphicsPipelineSetLayout);

	gEngine->GetAssetManager()->UnloadMesh(mMeshHandle);
}

void FRenderingTestLayer::BeginTick_GameThread(float deltaTime)
{
	ImGui::Begin("Rendering test");
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
	const THandle<FDescriptorPool> descriptorPool = gpu->GetDescriptorPool();
	const FGeometryBuffer& geometryBuffer = FGraphicsLocator::GetGeometryBuffer();

	const FSubMesh* mesh = gEngine->GetAssetManager()->AccessMesh(mMeshHandle);

	// Graphics pipeline
	const static FName graphicsDescriptorSetName("GraphicsSet");
	FDescriptorSetBuilder graphicsDescriptorSetBuilder;
	graphicsDescriptorSetBuilder
		.SetDescriptorPool(descriptorPool)
		.SetLayout(mGraphicsPipelineSetLayout)
		.SetBuffer(mesh->mPositionBuffer, 0)
		.SetName(graphicsDescriptorSetName);

	const THandle<FDescriptorSet> graphicsDescriptorSet = gpu->CreateDescriptorSet(graphicsDescriptorSetBuilder);

	const THandle<FTexture> drawImageHandle = geometryBuffer.GetColor();

	cmd->TransitionImage(drawImageHandle, vk::ImageLayout::eColorAttachmentOptimal);

	FRenderingAttachments renderingAttachments;
	renderingAttachments.AddColorAttachment(drawImageHandle);

	cmd->BeginRendering(renderingAttachments);
	cmd->SetViewport(FViewport::FromSize(geometryBuffer.GetResolution()));
	cmd->SetScissor(FRect2DInt::FromSize(geometryBuffer.GetResolution()));

	const glm::mat4 modelMat = glm::translate(glm::mat4(1.f), mModelLocation);

	glm::mat4 viewMat =
		glm::rotate(glm::mat4(1.f), glm::radians(mCameraRotation.x), EVec3::Right)
		* glm::rotate(glm::mat4(1.f), glm::radians(mCameraRotation.y), EVec3::Up)
		* glm::translate(glm::mat4(1.f), -mCameraLocation);

	// glm::mat4 viewMat = glm::lookAt(mCameraLocation, glm::vec3(0.f), EVec3::Up);


	const glm::vec2 resolution = geometryBuffer.GetResolution();
	glm::mat4 projMat = glm::perspectiveFov(glm::radians(mCameraFov), resolution.x, resolution.y, mCameraNearFar.x, mCameraNearFar.y);
	FMath::ConvertTurboToVulkanCoordinates(projMat);

	FPushConstants pushConstants = {};
	pushConstants.objToProj = projMat * viewMat * modelMat;
	pushConstants.objToView = modelMat;

	cmd->BindPipeline(mGraphicsPipeline);
	cmd->BindDescriptorSet(graphicsDescriptorSet, 0);
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