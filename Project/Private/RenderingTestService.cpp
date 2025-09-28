#include "imgui.h"
#include "RenderingTestLayer.h"

#include "Core/CoreTimer.h"
#include "Core/Engine.h"
#include "Graphics/GeometryBuffer.h"
#include "Graphics/GPUDevice.h"
#include "Graphics/GraphicsLocator.h"
#include "Graphics/ResourceBuilders.h"

using namespace Turbo;

struct FComputePushConstants
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

	mSetLayout = gpu->CreateDescriptorSetLayout(descriptorSetLayoutBuilder);

	FPipelineBuilder pipelineBuilder;
	pipelineBuilder.AddDescriptorSetLayout(mSetLayout);
	pipelineBuilder.SetPushConstantType<FComputePushConstants>();
	pipelineBuilder.GetShaderState().AddStage("Shader", vk::ShaderStageFlagBits::eCompute);

	mPipeline = gpu->CreatePipeline(pipelineBuilder);

	uniformBufferData.vertices[0] = glm::vec4(0.5f, 0.5f, 0.f, 0.f);
	uniformBufferData.vertices[1] = glm::vec4(0.2f, 0.8f, 0.f, 0.f);
	uniformBufferData.vertices[2] = glm::vec4(0.8f, 0.8f, 0.f, 0.f);

	static const FName kBufferName = FName("RenderingTestUniformBuffer");
	FBufferBuilder bufferBuilder;
	bufferBuilder
		.Init(vk::BufferUsageFlagBits::eUniformBuffer, EBufferFlags::CreateMapped, sizeof(FParametersBuffer))
		.SetData(&uniformBufferData)
		.SetName(kBufferName);

	mUniformBufferHandle = gpu->CreateBuffer(bufferBuilder);
}

void FRenderingTestLayer::Shutdown()
{
	FGPUDevice* gpu = gEngine->GetGpu();

	gpu->DestroyBuffer(mUniformBufferHandle);
	gpu->DestroyDescriptorSetLayout(mSetLayout);
	gpu->DestroyPipeline(mPipeline);
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
	const FDescriptorPoolHandle descriptorPool = gpu->GetDescriptorPool();
	const FGeometryBuffer& geometryBuffer = FGraphicsLocator::GetGeometryBuffer();

	FBuffer* uniformBuffer = gpu->AccessBuffer(mUniformBufferHandle);
	void* mappedAddress = uniformBuffer->GetMappedAddress();
	TURBO_CHECK(mappedAddress);

	std::memcpy(mappedAddress, &uniformBufferData, sizeof(FParametersBuffer));

	const FTextureHandle drawImage = geometryBuffer.GetColor();

	const static FName descriptorSetName("ComputeSet");
	FDescriptorSetBuilder descriptorSetBuilder;
	descriptorSetBuilder
		.SetDescriptorPool(descriptorPool)
		.SetLayout(mSetLayout)
		.SetTexture(drawImage, 0)
		.SetBuffer(mUniformBufferHandle, 1)
		.SetName(descriptorSetName);

	const FDescriptorSetHandle descriptorSet = gpu->CreateDescriptorSet(descriptorSetBuilder);

	cmd->TransitionImage(drawImage, vk::ImageLayout::eGeneral);
	cmd->BindPipeline(mPipeline);
	cmd->BindDescriptorSet(descriptorSet, 0);

	FComputePushConstants pushConstants = {};
	pushConstants.time = static_cast<float>(FCoreTimer::TimeFromEngineStart());

	cmd->PushConstants(pushConstants);

	const glm::ivec2& resolution = geometryBuffer.GetResolution();
	cmd->Dispatch(FMath::DivideAndRoundUp(glm::ivec3(resolution, 1), glm::ivec3(8, 8, 1)));
}

FName FRenderingTestLayer::GetName()
{
	const static FName kName = FName("RenderingTest");
	return kName;
}