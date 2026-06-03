#pragma once

#include "Core/DataStructures/Handle.h"
#include "Graphics/GPUDevice.h"
#include "Graphics/ResourceBuilders.h"
#include "Graphics/Resources.h"

namespace Turbo::ToneMapperPostProcess
{
	struct FComponent final
	{
		float mExposure = 0.f;
		float mSaturation = 1.f;

		glm::float3 mOffset = glm::float3(0.f);
		glm::float3 mSlope = glm::float3(1.f);
		glm::float3 mPower = glm::float3(1.f);
	};

	struct FUniformBuffer final
	{
		float mExposure;
		float mSaturation;

		glm::float3 mOffset;
		glm::float3 mSlope;
		glm::float3 mPower;
	};

	struct FPushConstants final
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
