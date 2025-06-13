#include "Core/RHI/ShaderProgram.h"

#include "Core/Engine.h"
#include "Core/RHI/VulkanDevice.h"

namespace Turbo {
	FShaderModule::FShaderModule(FVulkanDevice& Device)
		: mDevice(&Device)
	{
	}

	FShaderModule::~FShaderModule()
	{
		mDevice->Get().destroyShaderModule(mVulkanShaderModule);
	}

	void FShaderModule::Init(const std::vector<uint8>& shaderCode, EShaderType newShaderType)
	{
		vk::ShaderModuleCreateInfo createInfo;
		createInfo.codeSize = shaderCode.size();
		createInfo.pCode = reinterpret_cast<const uint32*>(shaderCode.data());

		vk::Result vulkanResult;
		std::tie(vulkanResult, mVulkanShaderModule) = mDevice->Get().createShaderModule(createInfo);
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