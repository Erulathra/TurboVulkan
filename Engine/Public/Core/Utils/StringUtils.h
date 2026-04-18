#pragma once
#include <string_view>

namespace StringUtils
{
	inline std::vector<std::string_view> SplitString(const std::string_view input, char delimiter)
	{
		std::vector<std::string_view> result;

		uint32 currentStartIndex = 0;
		for (int32 characterIndex = currentStartIndex; characterIndex < input.size(); ++characterIndex)
		{
			if (input[characterIndex] == delimiter)
			{
				const size_t partLength = characterIndex - currentStartIndex;
				if (partLength > 0)
				{
					result.emplace_back(&input[currentStartIndex], characterIndex - currentStartIndex);
				}

				currentStartIndex = characterIndex + 1;
			}
		}

		if (currentStartIndex < input.size())
		{
			result.emplace_back(&input[currentStartIndex], input.size() - currentStartIndex);
		}

		return result;
	}

	template<bool bCaseSensitive>
	bool CompareString(const std::string_view left, const std::string_view right)
	{
		if (left.length() != right.length())
		{
			return false;
		}

		for (size_t charId = 0; charId < left.length(); ++charId)
		{
			if (left[charId] != right[charId])
			{
				if constexpr (bCaseSensitive)
				{
					return false;
				}
				else
				{
					constexpr char caseOffset = 'a' - 'A';
					const char leftLower = (left[charId] >= 'A' && left[charId] <= 'Z')
						? static_cast<char>(left[charId] + caseOffset)
						: left[charId];
					const char rightLower = (right[charId] >= 'A' && right[charId] <= 'Z')
						? static_cast<char>(right[charId] + caseOffset)
						: right[charId];

					if (leftLower != rightLower)
					{
						return false;
					}
				}
			}
		}

		return true;
	}

	inline std::optional<bool> ParseBool(const std::string_view stringView)
	{
		std::optional<bool> result;

		if (CompareString<false>(stringView, "true") || CompareString<false>(stringView, "1"))
		{
			result = true;
		}
		else if (CompareString<false>(stringView, "false") || CompareString<false>(stringView, "0"))
		{
			result = false;
		}

		return result;
	}

	inline std::optional<int32> ParseInt(const std::string_view stringView)
	{
		std::optional<int32> result;

		int32 outInt;
		const std::from_chars_result parsingResult = std::from_chars(stringView.data(), stringView.data() + stringView.size(), outInt);
		if (parsingResult.ec != std::errc::invalid_argument && parsingResult.ec != std::errc::result_out_of_range)
		{
			result = outInt;
		}

		return result;
	};

	inline std::optional<float> ParseFloat(const std::string_view stringView)
	{
		std::optional<float> result;

		float outFloat;
		const std::from_chars_result parsingResult = std::from_chars(stringView.data(), stringView.data() + stringView.size(), outFloat);
		if (parsingResult.ec != std::errc::invalid_argument && parsingResult.ec != std::errc::result_out_of_range)
		{
			result = outFloat;
		}

		return result;
	};
}

namespace ASCII
{
	inline bool IsLetter(char character)
	{
		return (character >= 'A' && character <= 'Z') || (character >= 'a' && character <= 'z');
	}
}
