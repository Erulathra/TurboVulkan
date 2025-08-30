#include "Graphics/ShaderCompiler.h"

#include "SlangShaderCompiler.h"

namespace Turbo
{
	std::unique_ptr<IShaderCompiler> gShaderCompiler;

	IShaderCompiler& IShaderCompiler::Get()
	{
		if (!gShaderCompiler)
		{
			gShaderCompiler = std::make_unique<FSlangShaderCompiler>();
		}

		return *gShaderCompiler;
	}
} // Turbo