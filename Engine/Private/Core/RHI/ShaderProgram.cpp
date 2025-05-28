#include "Core/RHI/ShaderProgram.h"

#include "Core/Engine.h"
#include "Core/RHI/VulkanDevice.h"
#include "Core/RHI/VulkanRHI.h"

namespace Turbo {
	FShaderProgram::~FShaderProgram()
	{
		if (FVulkanDevice* DevicePtr = gEngine->GetRHI()->GetDevice())
		{
			vkDestroyShaderModule(DevicePtr->GetVulkanDevice(), ShaderModule, nullptr);
		}
	}

	void FShaderProgram::Init(const std::vector<uint8>& ShaderCode, const FVulkanDevice* InDevice)
	{
		VkShaderModuleCreateInfo CreateInfo;
		CreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		CreateInfo.codeSize = ShaderCode.size();
		CreateInfo.pCode = reinterpret_cast<const uint32*>(ShaderCode.data());

		const VkResult ModuleCreationResult = vkCreateShaderModule(InDevice->GetVulkanDevice(), &CreateInfo, nullptr, &ShaderModule);
		if (ModuleCreationResult != VK_SUCCESS)
		{
			TURBO_LOG(LOG_RHI, LOG_ERROR, "Shader Module Creation Error: {}", (int32)ModuleCreationResult);
		}
	}
} // Turbo