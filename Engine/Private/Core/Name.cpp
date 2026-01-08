#include "Core/Name.h"

namespace Turbo {
	using FNameHashMapType = std::unordered_map<std::string /** name */, FNameId /* id */>;
	using FNameLookUpTableType = std::vector<std::string>;

	TUniquePtr<FNameHashMapType> gNameHashMap;
	TUniquePtr<FNameLookUpTableType> gNameLookUp;

	uint32 TryRegisterName(std::string_view name)
	{
		const std::string nameAsString(name);

		if (!gNameHashMap)
		{
			gNameHashMap = std::make_unique<FNameHashMapType>();
			gNameLookUp = std::make_unique<FNameLookUpTableType>();
		}

		if (const auto foundNameIt = gNameHashMap->find(nameAsString); foundNameIt != gNameHashMap->end())
		{
			return foundNameIt->second;
		}

		const FNameId newId = gNameHashMap->size();
		(*gNameHashMap)[nameAsString] = newId;
		gNameLookUp->push_back(nameAsString);

		return newId;
	}

	FName::FName() : mStringId(kNameNone.mStringId)
	{
		static_assert(sizeof(FName) == 4);
	}
	FName::FName(std::string_view string)
	{
		mStringId = TryRegisterName(string);
	}

	bool FName::IsNone() const
	{
		return *this == kNameNone;
	}

	std::string_view FName::ToString() const
	{
		TURBO_CHECK(gNameLookUp->size() > mStringId);
		return (*gNameLookUp)[mStringId];
	}

	cstring FName::ToCString() const
	{
		TURBO_CHECK(gNameLookUp->size() > mStringId);
		return (*gNameLookUp)[mStringId].c_str();
	}
} // Turbo
