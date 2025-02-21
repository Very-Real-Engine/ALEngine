#pragma once

#include "Core/Base.h"
#include "Core/Log.h"
#include <filesystem>

/**
 * @brief Assert 매크로 헤더
 * 
 */

#ifdef AL_ENABLE_ASSERTS

// Alteratively we could use the same "default" message for both "WITH_MSG" and "NO_MSG" and
// provide support for custom formatting by concatenating the formatting string instead of having the format inside the
// default message
#define AL_INTERNAL_ASSERT_IMPL(type, check, msg, ...)                                                                 \
	{                                                                                                                  \
		if (!(check))                                                                                                  \
		{                                                                                                              \
			AL##type##ERROR(msg, __VA_ARGS__);                                                                         \
			AL_DEBUGBREAK();                                                                                           \
		}                                                                                                              \
	}
#define AL_INTERNAL_ASSERT_WITH_MSG(type, check, ...)                                                                  \
	AL_INTERNAL_ASSERT_IMPL(type, check, "Assertion failed: {0}", __VA_ARGS__)
#define AL_INTERNAL_ASSERT_NO_MSG(type, check)                                                                         \
	AL_INTERNAL_ASSERT_IMPL(type, check, "Assertion '{0}' failed at {1}:{2}", AL_STRINGIFY_MACRO(check),               \
							std::filesystem::path(__FILE__).filename().string(), __LINE__)

#define AL_INTERNAL_ASSERT_GET_MACRO_NAME(arg1, arg2, macro, ...) macro
#define AL_INTERNAL_ASSERT_GET_MACRO(...)                                                                              \
	AL_EXPAND_MACRO(                                                                                                   \
		AL_INTERNAL_ASSERT_GET_MACRO_NAME(__VA_ARGS__, AL_INTERNAL_ASSERT_WITH_MSG, AL_INTERNAL_ASSERT_NO_MSG))

// Currently accepts at least the condition and one additional parameter (the message) being optional
#define AL_ASSERT(...) AL_EXPAND_MACRO(AL_INTERNAL_ASSERT_GET_MACRO(__VA_ARGS__)(_, __VA_ARGS__))
#define AL_CORE_ASSERT(...) AL_EXPAND_MACRO(AL_INTERNAL_ASSERT_GET_MACRO(__VA_ARGS__)(_CORE_, __VA_ARGS__))
#else
#define AL_ASSERT(...)
#define AL_CORE_ASSERT(...)
#endif