#include "AssetLoading/MeshLoader.h"

#include "GLTFMeshLoader.h"

namespace Turbo {
	IMeshLoader* gInstance;

	IMeshLoader* IMeshLoader::Get()
	{
		if (gInstance == nullptr)
		{
			gInstance = new FGLTFMeshLoader();
		}

		return gInstance;
	}
} // Turbo