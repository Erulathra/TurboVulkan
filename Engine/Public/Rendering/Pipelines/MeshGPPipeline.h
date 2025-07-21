#pragma once

#include "Core/RHI/Pipelines/GraphicsPipelineBase.h"

namespace Turbo
{
	class FMeshGPPipeline : public FGraphicsPipelineBase
	{
	public:
		struct FPushConstants
		{
			glm::mat4 ViewMatrix{1.f};
			vk::DeviceAddress VertexBuffer = 0;
		};

	public:
		explicit FMeshGPPipeline(FVulkanDevice* device);
		virtual ~FMeshGPPipeline() override = default;

	protected:
		virtual std::vector<vk::PushConstantRange> InitPushConstantRanges() override;
	};
} // Turbo
