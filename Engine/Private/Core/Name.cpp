#include "Core/Name.h"
#include <algorithm>
#include <cctype>

namespace Turbo {
	using FNameHashMapType = std::unordered_map<std::string /** name */, FNameId /* id */>;
	std::unique_ptr<FNameHashMapType> gNameHashMap;

	uint32 TryRegisterName(std::string_view name)
	{
		if (!gNameHashMap)
		{
			gNameHashMap = std::make_unique<FNameHashMapType>();
		}

		std::string nameAsString;
		nameAsString.reserve(name.size());
		std::ranges::transform(
			name, std::back_inserter(nameAsString),
			[](const unsigned char character) { return std::tolower(character); });

		if (const auto foundNameIt = gNameHashMap->find(nameAsString); foundNameIt != gNameHashMap->end())
		{
			return foundNameIt->second;
		}

		const FNameId newId = gNameHashMap->size();
		(*gNameHashMap)[nameAsString] = newId;

		return newId;
	}

	FName::FName() : mStringId(kNameNone.mStringId) { }
	FName::FName(std::string_view string)
	{
		mStringId = TryRegisterName(string);
	}
} // Turbo