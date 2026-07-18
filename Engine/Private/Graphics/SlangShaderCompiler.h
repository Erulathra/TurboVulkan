#pragma once

#include "Graphics/ShaderCompiler.h"

#include "slang.h"
#include "slang-com-ptr.h"
#include "vulkan/vulkan.hpp"

DECLARE_LOG_CATEGORY(LogSlang, Display, Display)

#ifndef SHADER_DEBUG_SYMBOLS
#define SHADER_DEBUG_SYMBOLS !TURBO_BUILD_SHIPPING
#endif // defined SHADER_DEBUG_SYMBOLS

namespace Turbo
{
	struct FShaderStage;

	inline const std::string kShaderCachePath = "Saved/ShaderCache/"s + (SHADER_DEBUG_SYMBOLS ? "Debug" : "Release") + "/";

	struct FShaderCacheFileHeader
	{
      static constexpr uint32 kCurrentVersion = 0;

	   uint32 mHeaderVersion;
	   uint32 mFileSize;
	   uint64 mShaderWriteTimeStamp;
	};

	class FSlangShaderCompiler final
      : public IShaderCompiler
	{
	public:
		virtual void Init() override;
		virtual void Destroy() override;
		virtual void ClearCache() override;

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
