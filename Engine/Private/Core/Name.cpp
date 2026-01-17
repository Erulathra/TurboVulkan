#include "Core/Name.h"

namespace Turbo {
	using FNameHashMapType = entt::dense_map<size_t /* nameStringHash */, FNameId /* id */>;
	using FNameLookUpTableType = std::deque<std::string>;

	void FName::TryRegisterName(FName& outName, std::string_view sourceString)
	{
		static FNameHashMapType gNameHashMap = {};
		static FNameLookUpTableType gNameLookUp = {};
		static std::mutex gNameLookUpCS;

		const std::size_t sourceStringHash = std::hash<std::string_view>{}(sourceString);

		std::scoped_lock scopedLock(gNameLookUpCS);
		if (const auto foundNameIt = gNameHashMap.find(sourceStringHash);
			foundNameIt != gNameHashMap.end())
		{
			outName.mStringId = foundNameIt->second;
			outName.mStringPtr = &gNameLookUp[foundNameIt->second];

			return;
		}

		const FNameId newId = gNameHashMap.size();

		gNameHashMap[sourceStringHash] = newId;
		gNameLookUp.emplace_back(sourceString);

		outName.mStringId = newId;
		outName.mStringPtr = &gNameLookUp.back();
	}

	FName::FName()
		: mStringId(kNameNone.mStringId)
		, mStringPtr(kNameNone.mStringPtr)
	{
	}

	FName::FName(std::string_view string)
	{
		TryRegisterName(*this, string);
	}

	bool FName::IsNone() const
	{
		return *this == kNameNone;
	}

	std::string_view FName::ToString() const
	{
		return *mStringPtr;
	}

	cstring FName::ToCString() const
	{
		return mStringPtr->c_str();
	}

} // Turbo
