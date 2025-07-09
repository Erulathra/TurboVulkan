#include "Rendering/Pipelines/TestGPPipeline.h"

namespace Turbo {
	FTestGPPipeline::FTestGPPipeline(FVulkanDevice* device)
		: FGraphicsPipelineBase(device, "Shaders/GPShader.spv")
	{

	}
} // Turbo