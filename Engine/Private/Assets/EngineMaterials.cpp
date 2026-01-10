#include "Assets/EngineMaterials.h"

#include "Assets/MaterialManager.h"

namespace Turbo::EngineMaterials
{
	void InitEngineMaterials()
	{
		FMaterialManager& materialManager = entt::locator<FMaterialManager>::value();
		FPipelineBuilder pipelineBuilder = FMaterialManager::CreateOpaquePipeline("MeshTestMaterial.slang");
		materialManager.LoadMaterial<void, void>(kTriangleTest, pipelineBuilder, 0);

		pipelineBuilder = FMaterialManager::CreateOpaquePipeline("OpaqueBasePass.slang");
		materialManager.LoadMaterial<FBasePassMaterialData, FBasePassInstanceData>(kOpaqueBasePass, pipelineBuilder, 2048);
	}
}
