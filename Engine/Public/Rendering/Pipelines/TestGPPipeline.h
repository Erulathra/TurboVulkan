#pragma once

#include "Core/RHI/Pipelines/GraphicsPipelineBase.h"

namespace Turbo
{
	class FTestGPPipeline : public FGraphicsPipelineBase
	{
	public:
		FTestGPPipeline(FVulkanDevice* device);
		virtual ~FTestGPPipeline() = default;
	};

} // Turbo
