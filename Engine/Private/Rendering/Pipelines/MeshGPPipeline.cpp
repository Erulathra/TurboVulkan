#include "Rendering/Pipelines/MeshGPPipeline.h"

namespace Turbo {
	FMeshGPPipeline::FMeshGPPipeline(FVulkanDevice* device)
		: FGraphicsPipelineBase(device, "Shaders/MeshShader.spv")
	{
	}

	std::vector<vk::PushConstantRange> FMeshGPPipeline::InitPushConstantRanges()
	{
		vk::PushConstantRange pushConstantRange;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(FPushConstants);
		pushConstantRange.stageFlags = vk::ShaderStageFlagBits::eVertex;

		return {pushConstantRange};
	}

	vk::Format FMeshGPPipeline::GetDepthFormat()
	{
		return vk::Format::eD32Sfloat;
	}

	vk::PipelineDepthStencilStateCreateInfo FMeshGPPipeline::InitDepthStencilState()
	{
		vk::PipelineDepthStencilStateCreateInfo depthStencilState = Super::InitDepthStencilState();
		depthStencilState.setDepthTestEnable(true);
		depthStencilState.setDepthWriteEnable(true);
		depthStencilState.setDepthCompareOp(vk::CompareOp::eGreaterOrEqual);
		depthStencilState.setDepthBoundsTestEnable(false);

		return depthStencilState;
	}
} // Turbo