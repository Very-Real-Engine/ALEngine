#pragma once

/**
 * @file Log.h
 * @brief 로깅 시스템을 제공하는 클래스 및 매크로 정의
 *
 * 이 파일은 `spdlog`를 기반으로 한 로깅 시스템을 제공합니다.
 * `Log` 클래스를 통해 코어(core) 및 클라이언트(client) 로거를 설정하고 사용할 수 있습니다.
 * 또한 로깅을 쉽게 수행할 수 있도록 다양한 매크로를 정의합니다.
 */

#include "Core/Base.h"
#include "spdlog/fmt/ostr.h"
#include "spdlog/spdlog.h"
#include <memory>

namespace ale
{
/**
 * @class Log
 * @brief 애플리케이션 로깅 시스템 클래스
 *
 * `spdlog` 라이브러리를 활용하여 코어 및 클라이언트 로거를 관리하는 클래스입니다.
 * 애플리케이션 전반에서 로그를 기록할 수 있도록 싱글톤 패턴을 사용합니다.
 */
class Log
{
  public:
  	/**
     * @brief 로깅 시스템을 초기화합니다.
     *
     * 이 함수는 `spdlog`를 사용하여 코어 및 클라이언트 로거를 설정합니다.
     * 애플리케이션이 시작될 때 한 번 호출해야 합니다.
     */
	static void init();

	/**
     * @brief 코어 로거를 반환합니다.
     * 
     * @return `spdlog` 기반의 코어 로거 (`std::shared_ptr<spdlog::logger>` 참조 반환)
     */
	static std::shared_ptr<spdlog::logger> &getCoreLogger()
	{
		return s_CoreLogger;
	}

	/**
     * @brief 클라이언트 로거를 반환합니다.
     * 
     * @return `spdlog` 기반의 클라이언트 로거 (`std::shared_ptr<spdlog::logger>` 참조 반환)
     */
	static std::shared_ptr<spdlog::logger> &getClientLogger()
	{
		return s_ClientLogger;
	}

  private:
  	/**< 코어 로거 인스턴스 */
	static std::shared_ptr<spdlog::logger> s_CoreLogger;
	/**< 클라이언트 로거 인스턴스 */
	static std::shared_ptr<spdlog::logger> s_ClientLogger;
};
} // namespace ale

/**
 * @defgroup CoreLogMacros 코어 로그 매크로
 * @brief 코어 로거를 사용하여 로그를 기록하는 매크로
 * @{
 */
#define AL_CORE_TRACE(...) ::ale::Log::getCoreLogger()->trace(__VA_ARGS__)
#define AL_CORE_INFO(...) ::ale::Log::getCoreLogger()->info(__VA_ARGS__)
#define AL_CORE_WARN(...) ::ale::Log::getCoreLogger()->warn(__VA_ARGS__)
#define AL_CORE_ERROR(...) ::ale::Log::getCoreLogger()->error(__VA_ARGS__)
#define AL_CORE_FATAL(...) ::ale::Log::getCoreLogger()->fatal(__VA_ARGS__)

/**
 * @defgroup ClientLogMacros 클라이언트 로그 매크로
 * @brief 클라이언트 로거를 사용하여 로그를 기록하는 매크로
 * @{
 */
#define AL_TRACE(...) ::ale::Log::getClientLogger()->trace(__VA_ARGS__)
#define AL_INFO(...) ::ale::Log::getClientLogger()->info(__VA_ARGS__)
#define AL_WARN(...) ::ale::Log::getClientLogger()->warn(__VA_ARGS__)
#define AL_ERROR(...) ::ale::Log::getClientLogger()->error(__VA_ARGS__)
#define AL_FATAL(...) ::ale::Log::getClientLogger()->fatal(__VA_ARGS__)
