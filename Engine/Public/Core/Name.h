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

		FName& operator=(const FName& other)
		{
			mStringId = other.mStringId;
			return *this;
		}

		friend bool operator==(const FName& lhs, const FName& rhs) { return lhs.mStringId == rhs.mStringId; }
		friend bool operator!=(const FName& lhs, const FName& rhs) { return !(lhs == rhs); }

		[[nodiscard]] bool IsNone();

		[[nodiscard]] std::string_view ToString() const;
		[[nodiscard]] cstring ToCString() const;

	private:
		FNameId mStringId;

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
		return hash<Turbo::FNameId>()(name.mStringId);
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
