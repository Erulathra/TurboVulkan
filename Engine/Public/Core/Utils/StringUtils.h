#pragma once
#include <string_view>
#include <tuple>

namespace StringUtils
{
	inline std::tuple<std::string_view, std::string_view> SplitString(const std::string_view stringView, char delimiter)
	{
		std::tuple<std::string_view, std::string_view> result;
		std::get<0>(result) = stringView;

		for (uint32 characterId = 0; characterId < stringView.size(); ++characterId)
		{
			if (stringView[characterId] == delimiter)
			{
				const size_t secondStringStartPos = characterId == stringView.size() - 1 ? characterId : characterId + 1;

				result = std::make_tuple(
					std::string_view(stringView.substr(0, characterId)),
					std::string_view(stringView.substr(secondStringStartPos, stringView.size()))
				);

				break;
			}
		}

		return result;
	}

	inline bool StringViewToInt( const std::string_view stringView, uint32& outInt)
	{
		const std::from_chars_result result = std::from_chars(stringView.data(), stringView.data() + stringView.size(), outInt);
		return result.ec != std::errc::invalid_argument && result.ec != std::errc::result_out_of_range;
	};
}
