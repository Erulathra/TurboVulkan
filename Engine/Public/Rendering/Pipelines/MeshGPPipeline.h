#pragma once

#include "Core/RHI/Pipelines/GraphicsPipelineBase.h"

namespace Turbo
{
	class FMeshGPPipeline : public FGraphicsPipelineBase
	{
		GENERATED_BODY_MINIMAL(FMeshGPPipeline, FGraphicsPipelineBase);

	public:
		struct FPushConstants
		{
			glm::mat4 ViewMatrix{1.f};

			vk::DeviceAddress Positions = 0;
			vk::DeviceAddress Normals = 0;
			vk::DeviceAddress Colors = 0;
		};

	public:
		explicit FMeshGPPipeline(FVulkanDevice* device);
		virtual ~FMeshGPPipeline() override = default;

	protected:
		virtual std::vector<vk::PushConstantRange> InitPushConstantRanges() override;
		virtual vk::Format GetDepthFormat() override;
		virtual vk::PipelineDepthStencilStateCreateInfo InitDepthStencilState() override;
	};
} // Turbo
