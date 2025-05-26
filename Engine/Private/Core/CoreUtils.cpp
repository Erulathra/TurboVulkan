#include "Core/CoreUtils.h"

#include <fstream>

namespace Turbo {
	std::vector<byte> CoreUtils::ReadWholeFile(const std::string_view FilePath)
	{
		std::ifstream File(FilePath.data(), std::ios::ate | std::ios::binary);
		TURBO_CHECK(File.is_open());

		const size_t FileSize = File.tellg();
		std::vector<byte> Result(FileSize);
		File.seekg(0);
		File.read(reinterpret_cast<char*>(Result.data()), FileSize);
		File.close();

		return Result;
	}
} // Turbo