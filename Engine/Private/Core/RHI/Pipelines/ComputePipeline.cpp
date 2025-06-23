#include "Core/RHI/Pipelines/ComputePipeline.h"

namespace Turbo
{
	void FComputePipeline::SetDescriptors(const vk::DescriptorSetLayout& layout, const vk::DescriptorSet& set)
	{
		mDescriptorSetLayout = layout;
		mDescriptorSet = set;
	}
} // Turbo
