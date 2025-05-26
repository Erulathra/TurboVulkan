#include "Core/RHI/ShaderProgram.h"

#include "Core/RHI/LogicalDevice.h"

namespace Turbo {
	ShaderProgram::~ShaderProgram()
	{
		if (LogicalDevicePtr DevicePtr = WeakDevice.lock())
		{
			vkDestroyShaderModule(DevicePtr->GetVulkanDevice(), ShaderModule, nullptr);
		}
	}

	void ShaderProgram::Init(const std::vector<uint8>& ShaderCode, const LogicalDevicePtr& Device)
	{
		VkShaderModuleCreateInfo CreateInfo;
		CreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		CreateInfo.codeSize = ShaderCode.size();
		CreateInfo.pCode = reinterpret_cast<const uint32*>(ShaderCode.data());

		const VkResult ModuleCreationResult = vkCreateShaderModule(Device->GetVulkanDevice(), &CreateInfo, nullptr, &ShaderModule);
		if (ModuleCreationResult != VK_SUCCESS)
		{
			TURBO_LOG(LOG_RHI, LOG_ERROR, "Shader Module Creation Error: {}", (int32)ModuleCreationResult);
		}
	}
} // Turbo