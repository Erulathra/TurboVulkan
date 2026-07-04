#pragma once

namespace Turbo
{
	struct FShaderStage;

	class IShaderCompiler
	{
	public:
		static IShaderCompiler& Get();

		virtual void Init() = 0;
		virtual void Destroy() = 0;
		virtual void ClearCache() = 0;

		virtual vk::ShaderModule CompileShader(vk::Device device, const FShaderStage& shaderStage) = 0;
	};
} // Turbo
