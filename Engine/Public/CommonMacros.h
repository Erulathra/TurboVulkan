#pragma once

// Code assumes 64bit platform
#if !defined(WITH_TURBO_64) || !WITH_TURBO_64
#error 64 bit C++ compiler required.
#endif // !WITH_TURBO_64

// Version macros
#define MAKE_VERSION(Major, Minor, Patch) \
    ((((uint32_t)(Major)) << 22U) | (((uint32_t)(Minor)) << 12U) | ((uint32_t)(Patch)))

#define VERSION_MAJOR(version) ((uint32_t)(version) >> 22U)
#define VERSION_MINOR(version) (((uint32_t)(version) >> 12U) & 0x3FFU)
#define VERSION_PATCH(version) ((uint32_t)(version) & 0xFFFU)

#define TURBO_VERSION() MAKE_VERSION(TURBO_VERSION_MAJOR, TURBO_VERSION_MINOR, TURBO_VERSION_PATCH)

// Debug break
#if DEBUG
#if WITH_MSVC
#include <intrin.h>
#define TURBO_DEBUG_BREAK() __debugbreak()
#else
#include <csignal>
#define TURBO_DEBUG_BREAK() __builtin_debugtrap();
#endif
#else // DEBUG
#define TURBO_DEBUG_BREAK() {};
#endif // else DEBUG

// Assertions
#if WITH_ASSERTIONS

#define TURBO_CHECK(CONDITION)																	\
	if (!(CONDITION))																			\
	{																							\
		SPDLOG_ERROR("Assertion `" #CONDITION "` failed.");										\
		TURBO_DEBUG_BREAK();																	\
		std::terminate();																		\
	}

#define TURBO_CHECK_MSG(CONDITION, MESSAGE, ...)																		\
	if (!(CONDITION))																									\
	{																													\
		SPDLOG_ERROR("Assertion `" #CONDITION "` failed. Message: `" MESSAGE "`" __VA_OPT__(,) __VA_ARGS__);			\
		TURBO_DEBUG_BREAK();																							\
		std::terminate();																								\
	}

#else // WITH_ASSERTIONS

#define TURBO_CHECK(CONDITION) {}
#define TURBO_CHECK_MSG(CONDITION, MESSAGE, ...) {}

#endif // else WITH_ASSERTIONS

#define TURBO_UNINPLEMENTED()																							\
	{																													\
		SPDLOG_ERROR("Unimplemented behaviour.");																		\
		TURBO_DEBUG_BREAK();																							\
		std::terminate();																								\
	}																													\

#define TURBO_UNINPLEMENTED_MSG(MESSAGE, ...)																			\
	{																													\
		SPDLOG_ERROR("Unimplemented behaviour. Message: `" MESSAGE "`" __VA_OPT__(,) __VA_ARGS__);						\
		TURBO_DEBUG_BREAK();																							\
		std::terminate();																								\
	}																													\

#define TURBO_STATIC_ASSERT(CONDITION) static_assert(CONDITION, #CONDITION)
#define TURBO_STATIC_ASSERT_MSG(CONDITION, MSG) static_assert(CONDITION, "Static assert '" #CONDITION "' failed. Message: " MSG)

#if DEBUG

#define TURBO_ENSURE(CONDITION) \
if (!(CONDITION))				\
{								\
	TURBO_DEBUG_BREAK();		\
}								\

#else
#define TURBO_ENSURE(CONDITION) {}
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

#define EXPAND(X) X
#define GET_MACRO(_1, _2, NAME, ...) NAME
#define GENERATED_BODY(...) EXPAND( GET_MACRO(__VA_ARGS__, GENERATED_BODY_2, GENERATED_BODY_1)(__VA_ARGS__) )

#define GENERATED_BODY_MINIMAL(...) EXPAND( GET_MACRO(__VA_ARGS__, GENERATED_BODY_MINIMAL_2, GENERATED_BODY_MINIMAL_1)(__VA_ARGS__) )

#define GENERATED_BODY_MINIMAL_1(THIS_CLASS)						\
private:															\
	using ThisClass = THIS_CLASS;

#define GENERATED_BODY_MINIMAL_2(THIS_CLASS, SUPER_CLASS)			\
private:															\
	using ThisClass = THIS_CLASS;									\
	using Super = SUPER_CLASS;

#define GENERATED_BODY_1(THIS_CLASS)								\
public:																\
	static inline const FName mClassName =  FName (#THIS_CLASS);	\
GENERATED_BODY_MINIMAL(THIS_CLASS)

#define GENERATED_BODY_2(THIS_CLASS, SUPER_CLASS)					\
public:																\
	static inline const FName mClassName =  FName (#THIS_CLASS);	\
GENERATED_BODY_MINIMAL(THIS_CLASS, SUPER_CLASS)

#if WITH_PROFILER

#endif