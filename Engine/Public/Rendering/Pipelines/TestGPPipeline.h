#pragma once

#include "Core/RHI/Pipelines/GraphicsPipelineBase.h"

namespace Turbo
{
	class FTestGPPipeline : public FGraphicsPipelineBase
	{
	public:
		explicit FTestGPPipeline(FVulkanDevice* device);
		virtual ~FTestGPPipeline() override = default;
	};

} // Turbo
