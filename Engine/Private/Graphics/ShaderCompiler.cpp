#include "Graphics/ShaderCompiler.h"
#include "Graphics/SlangShaderCompiler.h"

namespace Turbo
{
	IShaderCompiler& IShaderCompiler::Get()
	{
   	static IShaderCompiler* gShaderCompiler = nullptr;
		if (gShaderCompiler == nullptr)
		{
			gShaderCompiler = new FSlangShaderCompiler();
		}

		return *gShaderCompiler;
	}
} // Turbo
