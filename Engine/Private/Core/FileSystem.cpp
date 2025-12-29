#include "Core/FileSystem.h"
#include <fstream>

namespace Turbo
{
	void FileSystem::LoadAssetData(FName filePath, std::vector<byte>& outData)
	{
		std::ifstream file(filePath.ToCString(), std::ios::in | std::ios::binary | std::ios::ate);
		TURBO_CHECK(file.is_open() && file.good())

		outData.resize(file.tellg());
		file.seekg(0);

		// I love you STL :)
		file.read(reinterpret_cast<char*>(outData.data()), static_cast<std::streamsize>(outData.size()));
	}
} // Turbo