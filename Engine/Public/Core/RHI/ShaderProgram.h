#pragma once

#include "RHICore.h"

namespace Turbo
{
	class FVulkanDevice;

	class FShaderProgram
	{
	public:
		FShaderProgram() = default;
		virtual ~FShaderProgram();

	public:
		void Init(const std::vector<uint8>& ShaderCode, const FVulkanDevice* InDevice);

	private:
		VkShaderModule ShaderModule = nullptr;
	};
} // Turbo
