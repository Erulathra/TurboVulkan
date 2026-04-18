#pragma once

namespace Turbo
{
	namespace FileSystem
	{
		constexpr std::string PathCombine(std::string_view lhs, std::string_view rhs)
		{
			std::string result;
			result.reserve(lhs.size() + rhs.size() + 2); // +2 because space and zero at end
			result.append(lhs);
			result.append("/");
			result.append(rhs);
			return result;
		}

		inline const std::string kSavedPath = "Saved";
		inline const std::string kShaderPath = "Shader";
		inline const std::string kLogPath = PathCombine(kSavedPath, "/Logs");
		inline const std::string kConfigPath = PathCombine(kSavedPath, "/Config");

		/** Use this to load asset data. It allows us to replace implementation
		 * to use zip pack instead of files in the future */
		bool LoadAssetData(FName filePath, std::vector<byte>& outData);
		bool LoadData(std::string_view path, std::vector<byte>& outData);

		bool CreateDirectory(std::string_view path);
		void InitDirectories();
	};
} // Turbo