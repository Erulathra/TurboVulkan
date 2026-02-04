#pragma once

namespace Turbo
{
	using FNameId = uint64;

	class FName
	{
	public:
		FName();
		explicit FName(std::string_view string);

		FName(const FName& other) : mStringId(other.mStringId), mStringPtr(other.mStringPtr) { }
		FName(FName&& other) noexcept : mStringId(other.mStringId), mStringPtr(other.mStringPtr) { }

		FName& operator=(const FName& other)
		{
			mStringId = other.mStringId;
			mStringPtr = other.mStringPtr;
			return *this;
		}

		friend bool operator==(const FName& lhs, const FName& rhs) { return lhs.mStringId == rhs.mStringId; }
		friend bool operator!=(const FName& lhs, const FName& rhs) { return !(lhs == rhs); }

		[[nodiscard]] bool IsNone() const;

		[[nodiscard]] std::string_view ToString() const;
		[[nodiscard]] cstring ToCString() const;

	private:
		static void TryRegisterName(FName& outName, std::string_view sourceString);

	private:
		FNameId mStringId;
		std::string* mStringPtr;

	public:
		friend class std::hash<FName>;
	};

	static const FName kNameNone = FName("none");
} // Turbo

template <>
struct std::hash<Turbo::FName>
{
	std::size_t operator()(const Turbo::FName& name) const noexcept
	{
		return name.mStringId;
	}
};

template <>
struct fmt::formatter<Turbo::FName> : fmt::formatter<std::string>
{
	auto format(const Turbo::FName& name, format_context& ctx) const
	{
		return formatter<std::string>::format(name.ToString(), ctx);
	}
};
