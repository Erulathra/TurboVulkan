#pragma once

#include "Core/RHI/RHICore.h"

namespace Turbo
{
	class FVulkanDevice;
	class FImage;

	class FDescriptorLayoutBuilder
	{
	public:
		FDescriptorLayoutBuilder& AddBinding(uint32 id, vk::DescriptorType type);
		FDescriptorLayoutBuilder& SetPNext(void* pNext);
		FDescriptorLayoutBuilder& SetFlags(vk::DescriptorSetLayoutCreateFlags flags);

		vk::DescriptorSetLayout Build(FVulkanDevice* device, vk::ShaderStageFlags shaderStage);
		void Clear();

	private:
		std::vector<vk::DescriptorSetLayoutBinding> mBindings {};
		vk::DescriptorSetLayoutCreateFlags mFlags;
		void* mPNext = nullptr;
	};
}
