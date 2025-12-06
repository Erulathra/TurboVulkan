#pragma once

#include "Assets/MaterialManager.h"

namespace Turbo
{
	struct FMesh;

	struct FMeshComponent final
	{
		THandle<FMesh> mMesh;
		THandle<FMaterial> mMaterial;
		THandle<FMaterial::Instance> mMaterialInstance;
	};
} // Turbo