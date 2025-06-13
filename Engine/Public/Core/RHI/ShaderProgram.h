#pragma once

#include "Core/RHI/RHICore.h"

namespace Turbo
{
	class FVulkanDevice;

	enum class EShaderType : uint8
	{
		None = 0,
		Vertex = 1 << 1,
		Fragment = 1 << 2
	};

	DEFINE_ENUM_OPERATORS(EShaderType);

	class FShaderModule
	{
	public:
		FShaderModule() = delete;
		FShaderModule(FVulkanDevice& Device);
		virtual ~FShaderModule();

	public:
		void Init(const std::vector<uint8>& shaderCode, EShaderType newShaderType);
		[[nodiscard]] std::vector<vk::PipelineShaderStageCreateInfo> GetShaderStageCreateInfo() const;

	private:
		FVulkanDevice* mDevice;

		EShaderType mShaderType = EShaderType::None;
		vk::ShaderModule mVulkanShaderModule{};
	};
} // Turbo

