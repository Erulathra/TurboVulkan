#pragma once

#include "Graphics/GraphicsCore.h"

namespace Turbo
{
	class FShaderStage;

	class IShaderCompiler
	{
	public:
		static IShaderCompiler& Get();

		virtual ~IShaderCompiler() = default;

		virtual void Init() = 0;
		virtual void Shutdown() = 0;

		virtual vk::ShaderModule CompileShader(vk::Device device, const FShaderStage& shaderStage) = 0;
	};
} // Turbo