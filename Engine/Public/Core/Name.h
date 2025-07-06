#pragma once

namespace Turbo
{
	using FNameId = uint32;

	class FName
	{
	public:
		explicit FName();
		explicit FName(std::string_view string);

		FName(const FName& other) : mStringId(other.mStringId) { }
		FName(FName&& other) noexcept : mStringId(other.mStringId) { }

		friend bool operator==(const FName& lhs, const FName& rhs) { return lhs.mStringId == rhs.mStringId; }
		friend bool operator!=(const FName& lhs, const FName& rhs) { return !(lhs == rhs); }

	private:
		FNameId mStringId;
	};

	static const FName kNameNone = FName("none");
} // Turbo
