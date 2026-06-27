#pragma once

#if defined(__clang__) == false
#error "Unsupported compiler"
#endif

using namespace std::string_literals;
using namespace std::string_view_literals;

// Version macros
#define MAKE_VERSION(Major, Minor, Patch) \
    ((((uint32_t)(Major)) << 22U) | (((uint32_t)(Minor)) << 12U) | ((uint32_t)(Patch)))

#define VERSION_MAJOR(version) ((uint32_t)(version) >> 22U)
#define VERSION_MINOR(version) (((uint32_t)(version) >> 12U) & 0x3FFU)
#define VERSION_PATCH(version) ((uint32_t)(version) & 0xFFFU)

#define TURBO_VERSION() MAKE_VERSION(TURBO_VERSION_MAJOR, TURBO_VERSION_MINOR, TURBO_VERSION_PATCH)

// Debug break
#if DEBUG
#define TURBO_DEBUG_BREAK() __builtin_debugtrap();
#else // DEBUG
#define TURBO_DEBUG_BREAK() {};
#endif // else DEBUG

#if defined(__clang__)
	#define TURBO_ASSUME(...) do { __builtin_assume(__VA_ARGS__); } while(0);
#elif defined(_MSC_VER)
	#define TURBO_ASSUME(...) do { __assume(__VA_ARGS__); } while(0);
#elif defined(__GNUC__)
#if __GNUC__ >= 13
	#define TURBO_ASSUME(...) __attribute__((__assume__(__VA_ARGS__)))
#endif
#endif

#ifdef __clang__
	#define DISABLE_ASSUME_WARNINGS					\
		_Pragma("clang diagnostic push")				\
		_Pragma("clang diagnostic ignored \"-Wassume\"")
	#define ENABLE_ASSUME_WARNINGS _Pragma("clang diagnostic pop")
#else
	#define DISABLE_ASSUME_WARNINGS {}
	#define ENABLE_ASSUME_WARNINGS {}
#endif //defined(__clang__)

// Assertions

#define TURBO_CHECK(CONDITION)																	\
	if (!(CONDITION)) [[unlikely]]																\
	{																							\
		SPDLOG_ERROR("Assertion `" #CONDITION "` failed.");										\
		TURBO_DEBUG_BREAK();																	\
		std::abort();																			\
		TURBO_ASSUME(false)																		\
	}																							\
	DISABLE_ASSUME_WARNINGS																		\
	TURBO_ASSUME(!!(CONDITION))																	\
	ENABLE_ASSUME_WARNINGS


#define TURBO_CHECK_MSG(CONDITION, MESSAGE, ...)																		\
	if (!(CONDITION)) [[unlikely]]																						\
	{																													\
		SPDLOG_ERROR("Assertion `" #CONDITION "` failed. Message: `" MESSAGE "`" __VA_OPT__(,) __VA_ARGS__);			\
		TURBO_DEBUG_BREAK();																							\
		std::abort();																									\
		TURBO_ASSUME(false)																								\
	}																													\
	DISABLE_ASSUME_WARNINGS																								\
	TURBO_ASSUME(!!(CONDITION))																							\
	ENABLE_ASSUME_WARNINGS


#if WITH_SLOW_ASSERTIONS
#define TURBO_CHECK_SLOW(CONDITION) TURBO_CHECK(CONDITION)
#define TURBO_CHECK_MSG_SLOW(CONDITION, MESSAGE, ...) TURBO_CHECK(CONDITION, MESSAGE __VA_OPT(,) __VA_ARGS__)
#else // WITH_SLOW_ASSERTIONS
#define TURBO_CHECK_SLOW(CONDITION) {}
#define TURBO_CHECK_MSG_SLOW(CONDITION, MESSAGE, ...) {}
#endif // else WITH_SLOW_ASSERTIONS

#define TURBO_STATIC_ASSERT(CONDITION) static_assert(CONDITION, #CONDITION)
#define TURBO_STATIC_ASSERT_MSG(CONDITION, MSG) static_assert(CONDITION, "Static assert '" #CONDITION "' failed. Message: " MSG)

#if DEBUG

#define TURBO_ENSURE(CONDITION) \
if (!(CONDITION))				\
{								\
	TURBO_DEBUG_BREAK();		\
}								\

#else
#define TURBO_ENSURE(CONDITION) (void)(CONDITION)
#endif // DEBUG


#define UNIMPLEMENTED_BODY()	\
	TURBO_DEBUG_BREAK();		\
	std::abort()

#define TURBO_UNINPLEMENTED()																						\
{																													\
	SPDLOG_ERROR("Unimplemented behaviour.");																		\
	UNIMPLEMENTED_BODY();																							\
}																													\

#define TURBO_UNINPLEMENTED_MSG(MESSAGE, ...)																		\
{																													\
	SPDLOG_ERROR("Unimplemented behaviour. Message: `" MESSAGE "`" __VA_OPT__(,) __VA_ARGS__);						\
	UNIMPLEMENTED_BODY();																							\
}

// Other

#define INDEX_NONE (-1)
#define TEXT(LITERAL_STRING) LITERAL_STRING

// #define TEST_FLAG(VALUE, FLAG) (static_cast<uint32>((VALUE) & (FLAG)) != 0)

namespace Turbo
{
	template <typename T>
	bool IsValid(const T* object)
	{
		return object && object->IsValid();
	}

	template <typename T>
	bool IsValid(const TSharedPtr<T>& object)
	{
		return object && object->IsValid();
	}

	template <typename T>
	bool IsValid(const TUniquePtr<T>& object)
	{
		return object && object->IsValid();
	}

	template <typename T>
	bool IsValidAndUnique(const TSharedPtr<T>& object)
	{
		return object && object->IsValid() && object.unique();
	}

	template <typename T>
	bool IsValid(const TWeakPtr<T>& object)
	{
		const TSharedPtr<const T> LockedObject = object.lock();
		return LockedObject && LockedObject->IsValid();
	}
}

#define DEFINE_ENUM_OPERATORS(ENUM_TYPE, INTEGER_TYPE)																														\
constexpr ENUM_TYPE operator~(ENUM_TYPE rhs) noexcept {	return static_cast<ENUM_TYPE>(~static_cast<INTEGER_TYPE>(rhs)); }												\
constexpr ENUM_TYPE operator|(ENUM_TYPE lhs, ENUM_TYPE rhs) noexcept { return static_cast<ENUM_TYPE>(static_cast<INTEGER_TYPE>(lhs) | static_cast<INTEGER_TYPE>(rhs)); }	\
constexpr ENUM_TYPE operator&(ENUM_TYPE lhs, ENUM_TYPE rhs) noexcept { return static_cast<ENUM_TYPE>(static_cast<INTEGER_TYPE>(lhs) & static_cast<INTEGER_TYPE>(rhs)); }	\
constexpr ENUM_TYPE operator^(ENUM_TYPE lhs, ENUM_TYPE rhs) noexcept { return static_cast<ENUM_TYPE>(static_cast<INTEGER_TYPE>(lhs) ^ static_cast<INTEGER_TYPE>(rhs)); }	\
constexpr ENUM_TYPE& operator|=(ENUM_TYPE& lhs, ENUM_TYPE rhs) noexcept { return lhs = (lhs | rhs); }															\
constexpr ENUM_TYPE& operator&=(ENUM_TYPE& lhs, ENUM_TYPE rhs) noexcept { return lhs = (lhs & rhs); }															\
constexpr ENUM_TYPE& operator^=(ENUM_TYPE& lhs, ENUM_TYPE rhs) noexcept { return lhs = (lhs ^ rhs); }

#define DELETE_COPY(CLASS_NAME)							\
	CLASS_NAME(CLASS_NAME const&) = delete;				\
	CLASS_NAME& operator=(CLASS_NAME const&) = delete;	\

#if WITH_PROFILER

#endif
