#pragma once

DECLARE_LOG_CATEGORY(LogGLTFSceneLoader, Display, Display)

namespace Turbo
{
	class FWorld;

	struct FGLTFSceneLoader
	{
		static void LoadGLTFScene(FWorld& world, FName path);
	};
} // Turbo