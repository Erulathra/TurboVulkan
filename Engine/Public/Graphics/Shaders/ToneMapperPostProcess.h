#pragma once

#include "Core/DataStructures/Handle.h"
#include "Graphics/GPUDevice.h"
#include "Graphics/ResourceBuilders.h"
#include "Graphics/Resources.h"

namespace Turbo::ToneMapperPostProcess
{
	struct FUniformBuffer
	{
   	float mExposure;
      float mOneOverPreExposure;

		float mSaturation;

		glm::float3 mOffset;
		glm::float3 mSlope;
		glm::float3 mPower;
	};

	struct FPushConstants
	{
		uint32 mSceneColor = kInvalidBinding;
		uint32 mOutput = kInvalidBinding;
		glm::uint2 mTextureSize = {};

		FDeviceAddress mUniforms = kNullDeviceAddress;
	};

	THandle<FPipeline> CreatePipeline();

	inline THandle<FPipeline> CreatePipeline(FGPUDevice& gpu)
	{
		FPipelineBuilder builder;
		builder
			.SetPushConstantType<FPushConstants>()
			.SetName(FName("ToneMapperPostProcess"));

		builder.mShaderStateBuilder
			.AddStage("PostProcess/ToneMapperPostProcess", vk::ShaderStageFlagBits::eCompute);

		return gpu.CreatePipeline(builder);
	}
}
