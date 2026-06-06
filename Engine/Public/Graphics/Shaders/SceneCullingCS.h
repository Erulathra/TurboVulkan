#pragma once

#include "Core/DataStructures/Handle.h"
#include "Graphics/GPUDevice.h"
#include "Graphics/ResourceBuilders.h"
#include "Graphics/Resources.h"

namespace Turbo::SceneCullingCS
{
	struct FPushConstants
	{
		FDeviceAddress mViewData;
		FDeviceAddress mDrawData;
		FDeviceAddress mBounds;

		FDeviceAddress mDrawIndirectCommand;

		uint mNumDraws;
	};

	inline THandle<FPipeline> CreatePipeline(FGPUDevice& gpu)
	{
		FPipelineBuilder pipelineBuilder = {};
		pipelineBuilder
			.SetPushConstantType<FPushConstants>()
			.SetName(FName("SceneCulling"));

		pipelineBuilder.mShaderStateBuilder
			.AddStage("SceneRendering/SceneCulling", vk::ShaderStageFlagBits::eCompute);

		return gpu.CreatePipeline(pipelineBuilder);
	}
}
