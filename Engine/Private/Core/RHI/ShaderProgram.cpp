#include "Core/RHI/ShaderProgram.h"

#include "Core/Engine.h"
#include "Core/RHI/VulkanDevice.h"

namespace Turbo {
	FShaderProgram::~FShaderProgram()
	{
		mDevice->GetVulkanDevice().destroyShaderModule(mVulkanShaderModule);
	}

	void FShaderProgram::Init(const std::vector<uint8>& ShaderCode, const FVulkanDevice* InDevice)
	{
		vk::ShaderModuleCreateInfo createInfo;
		createInfo.codeSize = shaderCode.size();
		createInfo.pCode = reinterpret_cast<const uint32*>(shaderCode.data());

		vk::Result vulkanResult;
		std::tie(vulkanResult, mVulkanShaderModule) = mDevice->GetVulkanDevice().createShaderModule(createInfo);
		CHECK_VULKAN_HPP(vulkanResult);

		mShaderType = newShaderType;
		TURBO_CHECK(mShaderType != EShaderType::None);
	}

	std::vector<vk::PipelineShaderStageCreateInfo> FShaderModule::GetShaderStageCreateInfo() const
	{
		std::vector<vk::PipelineShaderStageCreateInfo> result;

		vk::PipelineShaderStageCreateInfo stageCreateInfo{};
		stageCreateInfo.setModule(mVulkanShaderModule);

		if (TEST_FLAG(mShaderType, EShaderType::Vertex))
		{
			stageCreateInfo.setStage(vk::ShaderStageFlagBits::eVertex);
			stageCreateInfo.setPName("vsMain");

			result.push_back(stageCreateInfo);
		}

		if (TEST_FLAG(mShaderType, EShaderType::Vertex))
		{
			stageCreateInfo.setStage(vk::ShaderStageFlagBits::eFragment);
			stageCreateInfo.setPName("psMain");

			result.push_back(stageCreateInfo);
		}

		return result;
	}
} // Turbo