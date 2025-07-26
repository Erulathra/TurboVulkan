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
} // Turbo