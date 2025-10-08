#include "imgui.h"
#include "RenderingTestLayer.h"

#include "Core/CoreTimer.h"
#include "Core/Engine.h"
#include "Graphics/GeometryBuffer.h"
#include "Graphics/GPUDevice.h"
#include "Graphics/GraphicsLocator.h"
#include "Graphics/ResourceBuilders.h"

using namespace Turbo;

struct FPushConstants
{
	float time;
};

struct FParametersBuffer
{
	glm::vec4 vertices[3];
} uniformBufferData;

void FRenderingTestLayer::Start()
{
	FGPUDevice* gpu = gEngine->GetGpu();

	FDescriptorSetLayoutBuilder descriptorSetLayoutBuilder;
	descriptorSetLayoutBuilder
		.AddBinding(vk::DescriptorType::eStorageImage, 0)
		.AddBinding(vk::DescriptorType::eUniformBuffer, 1);

	mComputeSetLayout = gpu->CreateDescriptorSetLayout(descriptorSetLayoutBuilder);

	FPipelineBuilder computePipelineBuilder;
	computePipelineBuilder.AddDescriptorSetLayout(mComputeSetLayout);
	computePipelineBuilder.SetPushConstantType<FPushConstants>();
	computePipelineBuilder.GetShaderState().AddStage("Shader", vk::ShaderStageFlagBits::eCompute);
	computePipelineBuilder.SetName(FName("TestComputePipeline"));

	mComputePipeline = gpu->CreatePipeline(computePipelineBuilder);

	uniformBufferData.vertices[0] = glm::vec4(0.5f, 0.5f, 0.f, 0.f);
	uniformBufferData.vertices[1] = glm::vec4(0.2f, 0.8f, 0.f, 0.f);
	uniformBufferData.vertices[2] = glm::vec4(0.8f, 0.8f, 0.f, 0.f);

	static const FName kBufferName = FName("RenderingTestUniformBuffer");
	FBufferBuilder bufferBuilder;
	bufferBuilder
		.Init(vk::BufferUsageFlagBits::eUniformBuffer, EBufferFlags::CreateMapped, sizeof(FParametersBuffer))
		.SetData(&uniformBufferData)
		.SetName(kBufferName);

	mComputeUniformBufferHandle = gpu->CreateBuffer(bufferBuilder);

	// Graphics pipeline
	static const FName kRectVertName = FName("RectVertices");
	static const FName kRectIndicesName = FName("RectIndices");

	const static std::array kRectVertices = {
		glm::vec3(0.5f,-0.5f, 0.0f),
		glm::vec3(0.5f,0.5f, 0.0f),
		glm::vec3(-0.5f,-0.5f, 0.0f),
		glm::vec3(-0.5f,0.5f, 0.0f),
	};

	const static std::array kRectIndices = {
		0, 1, 2, 2, 1, 3
	};

	// Graphics pipeline

	bufferBuilder
		.Init(vk::BufferUsageFlagBits::eStorageBuffer, EBufferFlags::None, sizeof(kRectVertices))
		.SetData(kRectVertices.data())
		.SetName(kRectVertName);
	mMeshVertices = gpu->CreateBuffer(bufferBuilder);

	bufferBuilder
		.Init(vk::BufferUsageFlagBits::eStorageBuffer, EBufferFlags::None, sizeof(kRectIndices))
		.SetData(kRectIndices.data())
		.SetName(kRectIndicesName);
	mMeshIndices = gpu->CreateBuffer(bufferBuilder);

	FDescriptorSetLayoutBuilder descriptorSetBuilder;
	descriptorSetBuilder
		.AddBinding(vk::DescriptorType::eStorageBuffer, 0, FName("Vertices"))
		.AddBinding(vk::DescriptorType::eStorageBuffer, 1, FName("Colors"))
		.SetName(FName("GraphicsTest"));

	mGraphicsPipelineSetLayout = gpu->CreateDescriptorSetLayout(descriptorSetBuilder);

	FPipelineBuilder graphicsPipelineBuilder;
	graphicsPipelineBuilder.AddDescriptorSetLayout(mGraphicsPipelineSetLayout);
	graphicsPipelineBuilder.SetPushConstantType<FPushConstants>();
	graphicsPipelineBuilder.SetName(FName("TestGraphicsPipeline"));

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

	gpu->DestroyPipeline(mComputePipeline);
	gpu->DestroyDescriptorSetLayout(mComputeSetLayout);
	gpu->DestroyBuffer(mComputeUniformBufferHandle);

	gpu->DestroyPipeline(mGraphicsPipeline);
	gpu->DestroyDescriptorSetLayout(mGraphicsPipelineSetLayout);
	gpu->DestroyBuffer(mMeshVertices);
	gpu->DestroyBuffer(mMeshIndices);
}

void FRenderingTestLayer::BeginTick_GameThread(float deltaTime)
{
	ImGui::Begin("Rendering test");
	for (uint32 i = 0; i < 3; ++i)
	{
		ImGui::PushID(i);
		ImGui::DragFloat2("vert", glm::value_ptr(uniformBufferData.vertices[i]), 0.01f, 0.f, 1.f);
		ImGui::PopID();
	}
	ImGui::End();
}

void FRenderingTestLayer::PostBeginFrame_RenderThread(FGPUDevice* gpu, FCommandBuffer* cmd)
{
	const THandle<FDescriptorPool> descriptorPool = gpu->GetDescriptorPool();
	const FGeometryBuffer& geometryBuffer = FGraphicsLocator::GetGeometryBuffer();

	FBuffer* uniformBuffer = gpu->AccessBuffer(mComputeUniformBufferHandle);
	void* mappedAddress = uniformBuffer->GetMappedAddress();
	TURBO_CHECK(mappedAddress);

	std::memcpy(mappedAddress, &uniformBufferData, sizeof(FParametersBuffer));

	const THandle<FTexture> drawImageHandle = geometryBuffer.GetColor();

	const static FName descriptorSetName("ComputeSet");
	FDescriptorSetBuilder computeDescriptorSetBuilder;
	computeDescriptorSetBuilder
		.SetDescriptorPool(descriptorPool)
		.SetLayout(mComputeSetLayout)
		.SetTexture(drawImageHandle, 0)
		.SetBuffer(mComputeUniformBufferHandle, 1)
		.SetName(descriptorSetName);

	const THandle<FDescriptorSet> computeDescriptorSet = gpu->CreateDescriptorSet(computeDescriptorSetBuilder);

	cmd->TransitionImage(drawImageHandle, vk::ImageLayout::eGeneral);
	cmd->BindPipeline(mComputePipeline);
	cmd->BindDescriptorSet(computeDescriptorSet, 0);

	FPushConstants pushConstants = {};
	pushConstants.time = static_cast<float>(FCoreTimer::TimeFromEngineStart());

	cmd->PushConstants(pushConstants);

	const glm::ivec2& resolution = geometryBuffer.GetResolution();
	// cmd->Dispatch(FMath::DivideAndRoundUp(glm::ivec3(resolution, 1), glm::ivec3(8, 8, 1)));

	cmd->TransitionImage(drawImageHandle, vk::ImageLayout::eColorAttachmentOptimal);

	// Graphics pipeline
	const static FName graphicsDescriptorSetName("GraphicsSet");
	FDescriptorSetBuilder graphicsDescriptorSetBuilder;
	graphicsDescriptorSetBuilder
		.SetDescriptorPool(descriptorPool)
		.SetLayout(mGraphicsPipelineSetLayout)
		.SetBuffer(mMeshVertices, 0)
		.SetBuffer(mMeshIndices, 1)
		.SetName(graphicsDescriptorSetName);

	const THandle<FDescriptorSet> graphicsDescriptorSet = gpu->CreateDescriptorSet(graphicsDescriptorSetBuilder);

	FRenderingAttachments renderingAttachments;
	renderingAttachments.AddColorAttachment(drawImageHandle);

	cmd->BeginRendering(renderingAttachments);
	cmd->SetViewport(FViewport::FromSize(geometryBuffer.GetResolution()));
	cmd->SetScissor(FRect2DInt::FromSize(geometryBuffer.GetResolution()));

	cmd->BindPipeline(mGraphicsPipeline);
	cmd->BindDescriptorSet(graphicsDescriptorSet, 0);
	cmd->PushConstants(pushConstants);
	cmd->Draw(6);

	cmd->EndRendering();
}

FName FRenderingTestLayer::GetName()
{
	const static FName kName = FName("RenderingTest");
	return kName;
}