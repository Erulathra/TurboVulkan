#include "World/GLTFSceneLoader.h"

#include "Assets/GLTFHelpers.h"
#include "fastgltf/core.hpp"

namespace Turbo
{
	void LogGLTFError(std::string_view message, fastgltf::Error error)
	{
		TURBO_LOG(LogGLTFSceneLoader, Error, "{} Error: {} Message: {}", message, fastgltf::getErrorName(error), fastgltf::getErrorMessage(error));
	}

	void FGLTFSceneLoader::LoadGLTFScene(FWorld& world, FName path)
	{
		TURBO_LOG(LogGLTFSceneLoader, Info, "Loading {} scene using gltf scene loader.", path)

		FTurboGLTFDataBuffer dataBuffer = FTurboGLTFDataBuffer::Load(path);

		fastgltf::Parser parser;
		fastgltf::Expected<fastgltf::Asset> gltfAsset = parser.loadGltf(dataBuffer, path.ToCString(), fastgltf::Options::GenerateMeshIndices);
		if (gltfAsset.error() != fastgltf::Error::None)
		{
			LogGLTFError("Parsing error.", gltfAsset.error());
			return;
		}

		
	}
} // Turbo