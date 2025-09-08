#include "SlangShaderCompiler.h"

#include <filesystem>

#include "Graphics/ResourceBuilders.h"

namespace Turbo
{

	void FSlangShaderCompiler::Init()
	{
		TURBO_LOG(LOG_SLANG, Info, "Initializing Slang shader compiler.");

		slang::createGlobalSession(mGlobalSession.writeRef());
		CreateSession();
		PreloadModules();
	}

	void FSlangShaderCompiler::Destroy()
	{
		TURBO_LOG(LOG_SLANG, Info, "Destroying Slang shader compiler.");

		mSession = nullptr;
		mGlobalSession = nullptr;
		slang::shutdown();
	}

	vk::ShaderModule FSlangShaderCompiler::CompileShader(vk::Device device, const FShaderStage& shaderStage)
	{
		TURBO_LOG(LOG_SLANG, Info, "Compiling `Shaders/{}.slang` shader.", shaderStage.mShaderName);

		vk::ShaderModule result = nullptr;

		Slang::ComPtr<slang::IBlob> diagnosticsBlob;
		Slang::ComPtr<slang::IModule> module;
		const std::string targetShaderPath = fmt::format("Shaders/{}", shaderStage.mShaderName);
		module = mSession->loadModule(targetShaderPath.c_str(), diagnosticsBlob.writeRef());
		PrintMessageIfNeeded(diagnosticsBlob);

		if (!module)
		{
			TURBO_LOG(LOG_SLANG, Error, "Error during shader module creation.");
			return result;
		}

		Slang::ComPtr<slang::IEntryPoint> entryPoint;
		module->findEntryPointByName(shaderStage.mEntryPoint.c_str(), entryPoint.writeRef());
		if (!entryPoint)
		{
			TURBO_LOG(LOG_SLANG, Error, "Invalid or missing entry point.");
			return result;
		}

		std::vector<slang::IComponentType*> componentTypes = {module, entryPoint};

		Slang::ComPtr<slang::IComponentType> composedProgram;
		const SlangResult compilationResult = mSession->createCompositeComponentType(
			componentTypes.data(), static_cast<SlangInt>(componentTypes.size()),
			composedProgram.writeRef(),
			diagnosticsBlob.writeRef()
			);

		PrintMessageIfNeeded(diagnosticsBlob);
		if (compilationResult != SLANG_OK)
		{
			TURBO_LOG(LOG_SLANG, Error, "Error {} during shader program compilation.", compilationResult);
			return result;
		}

		TURBO_LOG(LOG_SLANG, Info, "Linking {} shader.", shaderStage.mShaderName);
		Slang::ComPtr<slang::IComponentType> linkedProgram;
		const SlangResult linkingResult = composedProgram->link(linkedProgram.writeRef(), diagnosticsBlob.writeRef());

		PrintMessageIfNeeded(diagnosticsBlob);
		if (linkingResult != SLANG_OK)
		{
			TURBO_LOG(LOG_SLANG, Error, "Error {} during shader program linking.", linkingResult);
			return result;
		}

		Slang::ComPtr<slang::IBlob> spirvCode;
		SlangResult finalResult = linkedProgram->getEntryPointCode(0, 0, spirvCode.writeRef(), diagnosticsBlob.writeRef());

		PrintMessageIfNeeded(diagnosticsBlob);
		if (finalResult != SLANG_OK)
		{
			TURBO_LOG(LOG_SLANG, Error, "Unknown shader compilation error ({})", finalResult);
			return result;
		}

		vk::ShaderModuleCreateInfo createInfo = {};
		createInfo.pCode = static_cast<const uint32_t*>(spirvCode->getBufferPointer());
		createInfo.codeSize = spirvCode->getBufferSize();

		CHECK_VULKAN_RESULT(result, device.createShaderModule(createInfo));

		return result;
	}

	void FSlangShaderCompiler::CreateSession()
	{
		slang::TargetDesc targetDesc = {};
		targetDesc.format = SLANG_SPIRV;
		targetDesc.profile = mGlobalSession->findProfile("glsl_460");

		slang::SessionDesc sessionDesc = {};

		constexpr std::array kShaderSearchPaths = {"Shaders/Modules"};
		sessionDesc.searchPaths = kShaderSearchPaths.data();
		sessionDesc.searchPathCount = kShaderSearchPaths.size();

		sessionDesc.targets = &targetDesc;
		sessionDesc.targetCount = 1;

		std::vector<slang::CompilerOptionEntry> compilerOptions = {
#if !TURBO_BUILD_SHIPPING
			{slang::CompilerOptionName::Optimization, {slang::CompilerOptionValueKind::Int, SLANG_OPTIMIZATION_LEVEL_NONE, 0, nullptr, nullptr}},
			{slang::CompilerOptionName::EmitSpirvDirectly, {slang::CompilerOptionValueKind::Int, 1, 0, nullptr, nullptr}},
			{slang::CompilerOptionName::DebugInformation, {slang::CompilerOptionValueKind::Int, SLANG_DEBUG_INFO_LEVEL_STANDARD, 0, nullptr, nullptr}}
#else
			{ slang::CompilerOptionName::Optimization, {slang::CompilerOptionValueKind::Int, SLANG_OPTIMIZATION_LEVEL_MAXIMAL, 0, nullptr, nullptr} },
#endif // !TURBO_BUILD_SHIPPING
		};

		sessionDesc.compilerOptionEntries = compilerOptions.data();
		sessionDesc.compilerOptionEntryCount = compilerOptions.size();

		mGlobalSession->createSession(sessionDesc, mSession.writeRef());
	}

	void FSlangShaderCompiler::PreloadModules()
	{
		// TODO: ???
	}

	void FSlangShaderCompiler::PrintMessageIfNeeded(slang::IBlob* diagnosticsBlob)
	{
		if (diagnosticsBlob)
		{
			TURBO_LOG(LOG_SLANG, Info, "There was a message compilation. Message: \n{}", static_cast<cstring>(diagnosticsBlob->getBufferPointer()));
		}
	}
} // Turbo