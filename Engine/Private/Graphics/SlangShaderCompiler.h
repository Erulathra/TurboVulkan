#pragma once

#include "Graphics/ShaderCompiler.h"

#include "slang.h"
#include "slang-com-ptr.h"

namespace Turbo
{
	class FShaderStage;

	class FSlangShaderCompiler final : public IShaderCompiler
	{
	public:
		virtual void Init() override;
		virtual void Destroy() override;

		virtual vk::ShaderModule CompileShader(vk::Device device, const FShaderStage& shaderStage) override;

	private:
		void CreateSession();
		void PreloadModules();

		void PrintMessageIfNeeded(slang::IBlob* diagnosticsBlob);

	private:
		Slang::ComPtr<slang::IGlobalSession> mGlobalSession;
		Slang::ComPtr<slang::ISession> mSession;
	};
} // Turbo