#pragma once

#include "Core/RHI/RHICore.h"

namespace Turbo
{
	class FDescriptorAllocatorStatic;
	class FVulkanDevice;

	class FComputePipeline
	{
	public:
		explicit FComputePipeline(FVulkanDevice* device)
			: mDevice(device)
		{
		}

		void Init(const vk::ShaderModule& shaderModule);
		void SetDescriptors(const vk::DescriptorSetLayout& layout, const vk::DescriptorSet& set);

		void Dispatch(const vk::CommandBuffer& cmd, glm::ivec3 groupCount);

		void Destroy();

	private:
		void CreatePipelineLayout();
		void CreatePipeline(const vk::ShaderModule& shaderModule);

	private:
		FVulkanDevice* mDevice;

		vk::Pipeline mPipeline = nullptr;
		vk::PipelineLayout mPipelineLayout = nullptr;

		vk::DescriptorSetLayout mDescriptorSetLayout = nullptr;
		vk::DescriptorSet mDescriptorSet = nullptr;
	};
} // Turbo
