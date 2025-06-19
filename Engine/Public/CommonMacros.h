#pragma once

// Code assumes 64bit platform
#if !defined(WITH_TURBO_64) || !WITH_TURBO_64
#error C++ compiler required.
#endif // !WITH_TURBO_64

// Version macros
#define MAKE_VERSION(Major, Minor, Patch) \
    ((((uint32_t)(Major)) << 22U) | (((uint32_t)(Minor)) << 12U) | ((uint32_t)(Patch)))

#define VERSION_MAJOR(version) ((uint32_t)(version) >> 22U)
#define VERSION_MINOR(version) (((uint32_t)(version) >> 12U) & 0x3FFU)
#define VERSION_PATCH(version) ((uint32_t)(version) & 0xFFFU)

#define TURBO_VERSION() MAKE_VERSION(TURBO_VERSION_MAJOR, TURBO_VERSION_MINOR, TURBO_VERSION_PATCH)

// Debug break
#if WITH_MSVC
#include <intrin.h>
#define TURBO_DEBUG_BREAK() __debugbreak()
#else
#include <signal.h>
#define TURBO_DEBUG_BREAK() raise(SIGTRAP);
#endif

// Assertions
#define TURBO_CHECK(CONDITION)																	\
	if (!(CONDITION))																			\
	{																							\
		SPDLOG_ERROR("Assertion `" #CONDITION "` failed.");										\
		TURBO_DEBUG_BREAK();																	\
		std::terminate();																		\
	}

#define TURBO_CHECK_MSG(CONDITION, MESSAGE, ...)																			\
	if (!(CONDITION))																										\
	{																														\
		SPDLOG_ERROR("Assertion `" #CONDITION "` failed. Message: `" MESSAGE "`" __VA_OPT__(,) __VA_ARGS__);				\
		TURBO_DEBUG_BREAK();																								\
		std::terminate();																									\
	}

#if DEBUG
namespace Turbo
{
	inline bool Ensure_Impl(bool bCondition)
	{
		if (!bCondition)
		{
			TURBO_DEBUG_BREAK();
		}

		return bCondition;
	}
}

#define TURBO_ENSURE(CONDITION) Ensure_Impl(CONDITION)
#else
#define TURBO_ENSURE(CONDITION) CONDITION
#endif // DEBUG

// Other

#define INDEX_NONE (-1)

#define TEST_FLAG(VALUE, FLAG) static_cast<uint32>(VALUE & FLAG) != 0

namespace Turbo
{
	template <typename T>
	bool IsValid(const T* object)
	{
		return object && object->IsValid();
	}

	template <typename T>
	bool IsValid(const std::shared_ptr<T>& object)
	{
		return object && object->IsValid();
	}

	template <typename T>
	bool IsValid(const std::unique_ptr<T>& object)
	{
		return object && object->IsValid();
	}

	template <typename T>
	bool IsValidAndUnique(const std::shared_ptr<T>& object)
	{
		return object && object->IsValid() && object.unique();
	}

	template <typename T>
	bool IsValid(const std::weak_ptr<T>& object)
	{
		const std::shared_ptr<const T> LockedObject = object.lock();
		return LockedObject && LockedObject->IsValid();
	}
}

#define DEFINE_ENUM_OPERATORS(ENUM_TYPE)																														\
constexpr ENUM_TYPE operator~(ENUM_TYPE rhs) noexcept {	return static_cast<ENUM_TYPE>(~static_cast<uint32>(rhs)); }												\
constexpr ENUM_TYPE operator|(ENUM_TYPE lhs, ENUM_TYPE rhs) noexcept { return static_cast<ENUM_TYPE>(static_cast<uint32>(lhs) | static_cast<uint32>(rhs)); }	\
constexpr ENUM_TYPE operator&(ENUM_TYPE lhs, ENUM_TYPE rhs) noexcept { return static_cast<ENUM_TYPE>(static_cast<uint32>(lhs) & static_cast<uint32>(rhs)); }	\
constexpr ENUM_TYPE operator^(ENUM_TYPE lhs, ENUM_TYPE rhs) noexcept { return static_cast<ENUM_TYPE>(static_cast<uint32>(lhs) ^ static_cast<uint32>(rhs)); }	\
constexpr ENUM_TYPE& operator|=(ENUM_TYPE& lhs, ENUM_TYPE rhs) noexcept { return lhs = (lhs | rhs); }															\
constexpr ENUM_TYPE& operator&=(ENUM_TYPE& lhs, ENUM_TYPE rhs) noexcept { return lhs = (lhs & rhs); }															\
constexpr ENUM_TYPE& operator^=(ENUM_TYPE& lhs, ENUM_TYPE rhs) noexcept { return lhs = (lhs ^ rhs); }
