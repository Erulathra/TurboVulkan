#include "Core/FileSystem.h"
#include <fstream>

namespace Turbo
{
	bool FileSystem::LoadAssetData(FName filePath, std::vector<byte>& outData)
	{
		return LoadData(filePath.ToString(), outData);
	}

	bool FileSystem::LoadData(std::string_view filePath, std::vector<byte>& outData)
	{
		std::ifstream file(std::string(filePath), std::ios::in | std::ios::binary | std::ios::ate);
		if (file.is_open() == false || file.good() == false)
		{
			return false;
		}

		outData.resize(file.tellg());
		file.seekg(0);

		// I love you STL :)
		file.read(reinterpret_cast<char*>(outData.data()), static_cast<std::streamsize>(outData.size()));
		return true;
	}
} // Turbo