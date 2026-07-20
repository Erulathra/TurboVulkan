#pragma once

#include <filesystem>
#include <string_view>
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

	class FSlangShaderCompiler final
      : public IShaderCompiler
	{
	public:
		inline static const std::filesystem::path kShaderCachePath = "Saved/ShaderCache/"s + (SHADER_DEBUG_SYMBOLS ? "Debug" : "Release") + "/";
		inline static const std::filesystem::path kShaderPath = "Shaders"s;
		inline static const std::string kShaderModuleExtension = ".slang-module"s;
		inline static const std::string kShaderExtension = ".slang"s;

	public:
		virtual void Init() override;
		virtual void Destroy() override;
		virtual void ClearRuntimeCache() override;

		virtual vk::ShaderModule CompileShader(vk::Device device, const FShaderStage& shaderStage) override;

	private:
		void CreateSession();
		void PreloadModules();

		void PrintMessageIfNeeded(slang::IBlob* diagnosticsBlob);

		std::filesystem::path GetCachedModulePath(const std::filesystem::path& basePath);

	private:
		Slang::ComPtr<slang::IGlobalSession> mGlobalSession;
		Slang::ComPtr<slang::ISession> mSession;

		std::set<std::string> mCachedModules;
	};
} // Turbo
