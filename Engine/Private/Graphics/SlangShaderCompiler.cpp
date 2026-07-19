#include "SlangShaderCompiler.h"

#include <filesystem>
#include <slang-com-ptr.h>
#include <slang.h>

#include "CommonMacros.h"
#include "Graphics/ResourceBuilders.h"
#include "Graphics/VulkanHelpers.h"
#include "ProfilingMacros.h"

namespace Turbo
{
	void FSlangShaderCompiler::Init()
	{
      TRACE_ZONE_SCOPED_N("Slang: Create global session");
		TURBO_LOG(LogSlang, Info, "Initializing Slang shader compiler.");

		slang::createGlobalSession(mGlobalSession.writeRef());

		CreateSession();
	}

	void FSlangShaderCompiler::Destroy()
	{
		TURBO_LOG(LogSlang, Info, "Destroying Slang shader compiler.");

		mSession = nullptr;
		mGlobalSession = nullptr;
		slang::shutdown();
	}

	void FSlangShaderCompiler::ClearRuntimeCache()
	{
		mSession = nullptr;
		CreateSession();

		mCachedModules.clear();
	}

	vk::ShaderModule FSlangShaderCompiler::CompileShader(vk::Device device, const FShaderStage& shaderStage)
	{
		TRACE_ZONE_SCOPED_FORMAT(CompileShader, "Compile Shader {} {}", shaderStage.mShaderName, shaderStage.mEntryPoint)
		TURBO_LOG(
			LogSlang, Info, "Compiling `{}` ({}) shader.", shaderStage.mShaderName, VulkanEnum::GetShaderStageName(shaderStage.mStage));

		vk::ShaderModule result = nullptr;

		Slang::ComPtr<slang::IBlob> diagnosticsBlob;
		Slang::ComPtr<slang::IModule> module;
		bool bRuntimeShaderCacheInvalid = false;

		{
			TRACE_ZONE_SCOPED_N("Load Module")

			module = mSession->loadModule(shaderStage.mShaderName.c_str(), diagnosticsBlob.writeRef());
			PrintMessageIfNeeded(diagnosticsBlob);

			if (!module)
			{
				TURBO_LOG(LogSlang, Error, "Error during shader module creation.");
				return result;
			}

			{
				// Cache loaded modues
				for (uint32 moduleId = 0; moduleId < mSession->getLoadedModuleCount(); ++moduleId)
				{
					slang::IModule* moduleToCache = mSession->getLoadedModule(moduleId);
					const std::filesystem::path loadedModulePath = moduleToCache->getFilePath();
					if (loadedModulePath.extension() == kShaderExtension)
					{
          		   TRACE_ZONE_SCOPED_N("Save cached shader")

                  const std::filesystem::path cachedModulePath = GetCachedModulePath(loadedModulePath);
						TURBO_LOG(LogSlang, Info, "Saving compiled `{}` to `{}`", loadedModulePath, cachedModulePath);

						const std::filesystem::path cachedModuleDir = cachedModulePath.parent_path();
						std::filesystem::create_directories(cachedModuleDir);

						moduleToCache->writeToFile(cachedModulePath.c_str());
						bRuntimeShaderCacheInvalid = true;
					}

					mCachedModules.emplace(loadedModulePath);
				}
			}

			const std::string loadedShaderPath = module->getFilePath();
			TURBO_LOG(LogSlang, Info, "Shader `{}` loaded from `{}`.", shaderStage.mShaderName, loadedShaderPath);
		}

		Slang::ComPtr<slang::IComponentType> composedProgram;
		{
			TRACE_ZONE_SCOPED_N("Compose and compile program")

			Slang::ComPtr<slang::IEntryPoint> entryPoint;
			module->findEntryPointByName(shaderStage.mEntryPoint.c_str(), entryPoint.writeRef());
			if (!entryPoint)
			{
				TURBO_LOG(LogSlang, Error, "Invalid or missing entry point.");
				return result;
			}

			std::vector<slang::IComponentType*> componentTypes = {module, entryPoint};

			const SlangResult compilationResult = mSession->createCompositeComponentType(componentTypes.data(),
																						 static_cast<SlangInt>(componentTypes.size()),
																						 composedProgram.writeRef(),
																						 diagnosticsBlob.writeRef());

			PrintMessageIfNeeded(diagnosticsBlob);
			if (compilationResult != SLANG_OK)
			{
				TURBO_LOG(LogSlang, Error, "Error {} during shader program compilation.", compilationResult);
				return result;
			}
		}

		Slang::ComPtr<slang::IComponentType> linkedProgram;
		{
			TRACE_ZONE_SCOPED_N("Link composed program")
			TURBO_LOG(LogSlang, Info, "Linking {} shader.", shaderStage.mShaderName);
			const SlangResult linkingResult = composedProgram->link(linkedProgram.writeRef(), diagnosticsBlob.writeRef());

			PrintMessageIfNeeded(diagnosticsBlob);
			if (linkingResult != SLANG_OK)
			{
				TURBO_LOG(LogSlang, Error, "Error {} during shader program linking.", linkingResult);
				return result;
			}
		}

		{
			TRACE_ZONE_SCOPED_N("Compile SPIRV shader to driver format")

			Slang::ComPtr<slang::IBlob> spirvCode;
			SlangResult finalResult = linkedProgram->getEntryPointCode(0, 0, spirvCode.writeRef(), diagnosticsBlob.writeRef());

			PrintMessageIfNeeded(diagnosticsBlob);
			if (finalResult != SLANG_OK)
			{
				TURBO_LOG(LogSlang, Error, "Unknown shader compilation error ({})", finalResult);
				return result;
			}

			vk::ShaderModuleCreateInfo createInfo = {};
			createInfo.pCode = static_cast<const uint32_t*>(spirvCode->getBufferPointer());
			createInfo.codeSize = spirvCode->getBufferSize();

			CHECK_VULKAN_RESULT(result, device.createShaderModule(createInfo));
		}

		// We need to clear runtime cache if any module was saved to persistent cache, to avoid ABI problems.
		if (bRuntimeShaderCacheInvalid)
		{
   		ClearRuntimeCache();
		}

		return result;
	}

	void FSlangShaderCompiler::CreateSession()
	{
   	TRACE_ZONE_SCOPED_N("Slang: Create session");

		slang::TargetDesc targetDesc = {};
		targetDesc.format = SLANG_SPIRV;
		targetDesc.profile = mGlobalSession->findProfile("spirv_1_6");
		targetDesc.forceGLSLScalarBufferLayout = true;

		slang::SessionDesc sessionDesc = {};

		const std::array<const char*, 2> kShaderSearchPaths = {kShaderCachePath.c_str(), kShaderPath.c_str()};
		sessionDesc.searchPaths = kShaderSearchPaths.data();
		sessionDesc.searchPathCount = kShaderSearchPaths.size();

		sessionDesc.targets = &targetDesc;
		sessionDesc.targetCount = 1;

		std::vector<slang::CompilerOptionEntry> compilerOptions  = {
   		{ slang::CompilerOptionName::UseUpToDateBinaryModule, {slang::CompilerOptionValueKind::Int, 1, 0, nullptr, nullptr} },

#if SHADER_DEBUG_SYMBOLS
			{slang::CompilerOptionName::Optimization, {slang::CompilerOptionValueKind::Int, SLANG_OPTIMIZATION_LEVEL_NONE, 0, nullptr, nullptr}},
			{slang::CompilerOptionName::EmitSpirvDirectly, {slang::CompilerOptionValueKind::Int, 1, 0, nullptr, nullptr}},
			{slang::CompilerOptionName::DebugInformation, {slang::CompilerOptionValueKind::Int, SLANG_DEBUG_INFO_LEVEL_STANDARD, 0, nullptr, nullptr}},
#else // SHADER_DEBUG_SYMBOLS
			{ slang::CompilerOptionName::Optimization, {slang::CompilerOptionValueKind::Int, SLANG_OPTIMIZATION_LEVEL_MAXIMAL, 0, nullptr, nullptr} },
#endif // else SHADER_DEBUG_SYMBOLS
		};

		sessionDesc.compilerOptionEntries = compilerOptions.data();
		sessionDesc.compilerOptionEntryCount = compilerOptions.size();

		mGlobalSession->createSession(sessionDesc, mSession.writeRef());
	}

	void FSlangShaderCompiler::PrintMessageIfNeeded(slang::IBlob* diagnosticsBlob)
	{
		if (diagnosticsBlob)
		{
			TURBO_LOG(LogSlang, Info, "Message: \n{}", static_cast<cstring>(diagnosticsBlob->getBufferPointer()));
		}
	}

	std::filesystem::path FSlangShaderCompiler::GetCachedModulePath(const std::filesystem::path& shaderSourcePath)
	{
		TURBO_CHECK(shaderSourcePath.extension() == kShaderExtension)

		const std::filesystem::path relativeSourcePath = std::filesystem::relative(shaderSourcePath, kShaderPath);
		std::filesystem::path cachePath = kShaderCachePath / relativeSourcePath;
		cachePath.replace_extension(kShaderModuleExtension);

		return cachePath;
	}
} // Turbo
