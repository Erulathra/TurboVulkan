#pragma once

#include <fstream>

namespace Turbo
{
	namespace FCoreUtils
	{
		template <typename ChunkType = byte>
		std::vector<ChunkType> ReadWholeFile(std::string_view path)
		{
			std::vector<ChunkType> result;

			std::ifstream file(path.data(), std::ios::ate | std::ios::binary);
			if (!file.is_open())
			{
				return result;
			}

			const size_t fileSize = file.tellg();
			result.resize(std::ceil(static_cast<float>(fileSize) / sizeof(ChunkType)));
			file.seekg(0);
			file.read(reinterpret_cast<char*>(result.data()), static_cast<long>(fileSize));
			file.close();

			return result;
		}
	};
} // Turbo
