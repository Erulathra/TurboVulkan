#include "Graphics/ShaderCompiler.h"

#include "SlangShaderCompiler.h"

namespace Turbo
{
	TUniquePtr<IShaderCompiler> gShaderCompiler;

	IShaderCompiler& IShaderCompiler::Get()
	{
		if (!gShaderCompiler)
		{
			gShaderCompiler = std::make_unique<FSlangShaderCompiler>();
		}

		return *gShaderCompiler;
	}
} // Turbo