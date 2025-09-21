#include "RenderingTestLayer.h"

#include "Core/CoreTimer.h"
#include "Core/Engine.h"
#include "Graphics/GeometryBuffer.h"
#include "Graphics/GPUDevice.h"
#include "Graphics/GraphicsLocator.h"
#include "Graphics/ResourceBuilders.h"

using namespace Turbo;

struct alignas(16) FComputePushConstants
{
	glm::vec4 ColorOne;
	glm::vec4 ColorTwo;
	float Time;
};

void FRenderingTestLayer::Start()
{
	FGPUDevice* gpu = gEngine->GetGpu();

	FDescriptorSetLayoutBuilder descriptorSetLayoutBuilder;
	descriptorSetLayoutBuilder
		.AddBinding(vk::DescriptorType::eStorageImage, 0);

	mSetLayout = gpu->CreateDescriptorSetLayout(descriptorSetLayoutBuilder);

	FPipelineBuilder pipelineBuilder;
	pipelineBuilder.AddDescriptorSetLayout(mSetLayout);
	pipelineBuilder.SetPushConstantType<FComputePushConstants>();
	pipelineBuilder.GetShaderState().AddStage("Shader", vk::ShaderStageFlagBits::eCompute);

	mPipeline = gpu->CreatePipeline(pipelineBuilder);
}

void FRenderingTestLayer::Shutdown()
{
	FGPUDevice* gpu = gEngine->GetGpu();

	gpu->DestroyDescriptorSetLayout(mSetLayout);
	gpu->DestroyPipeline(mPipeline);
}

void FRenderingTestLayer::PostBeginFrame_RenderThread(FGPUDevice* gpu, FCommandBuffer* cmd)
{
	const FDescriptorPoolHandle descriptorPool = gpu->GetDescriptorPool();
	const FGeometryBuffer& geometryBuffer = FGraphicsLocator::GetGeometryBuffer();

	const FTextureHandle drawImage = geometryBuffer.GetColor();

	const static FName descriptorSetName("ComputeSet");
	FDescriptorSetBuilder descriptorSetBuilder;
	descriptorSetBuilder
		.SetDescriptorPool(descriptorPool)
		.SetLayout(mSetLayout)
		.SetTexture(drawImage, 0)
		.SetName(descriptorSetName);

	const FDescriptorSetHandle descriptorSet = gpu->CreateDescriptorSet(descriptorSetBuilder);

	cmd->TransitionImage(drawImage, vk::ImageLayout::eGeneral);
	cmd->BindPipeline(mPipeline);
	cmd->BindDescriptorSet(descriptorSet, 0);

	FComputePushConstants pushConstants = {};
	pushConstants.Time = static_cast<float>(FCoreTimer::TimeFromEngineStart());
	pushConstants.ColorOne = ELinearColor::kCyan;
	pushConstants.ColorTwo = ELinearColor::kGreen;

	cmd->PushConstants(pushConstants);

	const glm::ivec2& resolution = geometryBuffer.GetResolution();
	cmd->Dispatch(FMath::DivideAndRoundUp(glm::ivec3(resolution, 1), glm::ivec3(8, 8, 1)));
}

FName FRenderingTestLayer::GetName()
{
	const static FName kName = FName("RenderingTest");
	return kName;
}