#pragma once

#include "AssetLoading/MeshLoader.h"

#include <fastgltf/core.hpp>

namespace Turbo
{
	class FGLTFMeshLoader final : public IMeshLoader
	{
	public:
		virtual ~FGLTFMeshLoader() override;

	public:
		virtual std::shared_ptr<FRHIMesh> LoadMesh(FVulkanDevice* device, const std::filesystem::path& path, ELoadMeshFlags loadMeshFlags) override;

	private:
		fastgltf::Parser mParser;
	};
}
