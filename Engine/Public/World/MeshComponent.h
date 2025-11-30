#pragma once

namespace Turbo
{
	struct FMesh;

	struct FMeshComponent final
	{
		THandle<FMesh> Mesh;
	};
} // Turbo