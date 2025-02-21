#pragma once

/**
 * @brief 자주 사용하는 매크로 헤더 
 * 
 */

#define AL_EXPAND_MACRO(x) x
#define AL_STRINGIFY_MACRO(x) #x

#define BIT(x) (1 << x)

#define AL_BIND_EVENT_FN(fn)                                                                                           \
	[this](auto &&...args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }
