#include "Core/RHI/Pipelines/DescriptorLayoutBuilder.h"

#include "Core/RHI/VulkanDevice.h"

using namespace Turbo;

FDescriptorLayoutBuilder& FDescriptorLayoutBuilder::AddBinding(uint32 id, vk::DescriptorType type)
{
	vk::DescriptorSetLayoutBinding binding{};
	binding.setBinding(id);
	binding.setDescriptorCount(1);
	binding.setDescriptorType(type);

	mBindings.push_back(binding);

	return *this;
}

FDescriptorLayoutBuilder& FDescriptorLayoutBuilder::SetPNext(void* pNext)
{
	mPNext = pNext;
	return *this;
}

FDescriptorLayoutBuilder& FDescriptorLayoutBuilder::SetFlags(vk::DescriptorSetLayoutCreateFlags flags)
{
	mFlags = flags;
	return *this;
}

void FDescriptorLayoutBuilder::Clear()
{
	mBindings.clear();
}

vk::DescriptorSetLayout FDescriptorLayoutBuilder::Build(FVulkanDevice* device, vk::ShaderStageFlags shaderStage)
{
	for (vk::DescriptorSetLayoutBinding& binding : mBindings)
	{
		binding.stageFlags |= shaderStage;
	}

	vk::DescriptorSetLayoutCreateInfo createInfo;
	createInfo.setPNext(mPNext);

	createInfo.setBindings(mBindings);
	createInfo.setFlags(mFlags);

	vk::Result result;
	vk::DescriptorSetLayout descriptorSetLayout;
	std::tie(result, descriptorSetLayout) = device->Get().createDescriptorSetLayout(createInfo);

	CHECK_VULKAN_HPP(result);

	return descriptorSetLayout;
}
