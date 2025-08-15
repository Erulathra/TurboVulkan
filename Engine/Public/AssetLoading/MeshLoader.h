#pragma once

#include <filesystem>

#include "Core/RHI/VulkanDevice.h"

namespace Turbo
{
	class FRHIMesh;

	enum class ELoadMeshFlags : uint8
	{
		None = 0,

		LoadPositions		= 1 << 0,
		LoadUV				= 1 << 1,
		LoadNormals			= 1 << 2,
		LoadColor			= 1 << 3,

		LoadVertex = LoadPositions | LoadUV | LoadNormals,
		LoadVertexWithColor = LoadVertex | LoadColor
	};
	DEFINE_ENUM_OPERATORS(ELoadMeshFlags, uint8)

	class IMeshLoader
	{
	public:
		static IMeshLoader* Get();

	public:
		virtual ~IMeshLoader() = default;

	public:
		virtual std::shared_ptr<FRHIMesh> LoadMesh(FVulkanDevice* device, const std::filesystem::path& path, ELoadMeshFlags loadMeshFlags) = 0;
	};
} // Turbo
