#pragma once

namespace Turbo
{
	namespace FileSystem
	{
		/** Use this to load asset data. It allows us to replace implementation
		 * to use zip pack instead of files in the future */
		void LoadAssetData(FName filePath, std::vector<byte>& outData);
		void LoadData(std::string_view path, std::vector<byte>& outData);
	};
} // Turbo