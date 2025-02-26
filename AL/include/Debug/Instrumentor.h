#pragma once

/**
 * @file Instrumentor.h
 * @brief 성능 분석을 위한 프로파일링 유틸리티
 *
 * 이 파일은 성능 프로파일링을 위한 `Instrumentor` 및 `InstrumentationTimer` 클래스를 정의합니다.
 * 코드 실행 시간을 측정하고 JSON 형식의 결과 파일을 생성하여 성능 분석을 도와줍니다.
 */

#include "Core/Base.h"
#include "Core/Log.h"

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>

namespace ale
{
/**
 * @typedef FloatingPointMicroseconds
 * @brief 마이크로초 단위의 부동소수점 지속시간 타입
 */
using FloatingPointMicroseconds = std::chrono::duration<double, std::micro>;

/**
 * @struct ProfileResult
 * @brief 개별 함수 또는 코드 블록의 프로파일링 결과를 저장하는 구조체
 */
struct ProfileResult
{
	std::string Name;

	FloatingPointMicroseconds Start;
	std::chrono::microseconds ElapsedTime;
	std::thread::id ThreadID;
};

/**
 * @struct InstrumentationSession
 * @brief 프로파일링 세션을 나타내는 구조체
 */
struct InstrumentationSession
{
	std::string Name;
};

/**
 * @class Instrumentor
 * @brief 성능 분석을 위한 프로파일러 클래스
 *
 * `Instrumentor` 클래스는 실행 시간을 측정하고 JSON 형식으로 결과를 저장하는 역할을 합니다.
 * 싱글톤 패턴으로 구현되었으며, 여러 코드 블록을 프로파일링할 수 있습니다.
 */
class Instrumentor
{
  public:
	Instrumentor(const Instrumentor &) = delete;
	Instrumentor(Instrumentor &&) = delete;

	/**
	 * @brief 새로운 프로파일링 세션을 시작합니다.
	 *
	 * @param name 세션 이름
	 * @param filepath 결과를 저장할 파일 경로 (기본값: "results.json")
	 */
	void beginSession(const std::string &name, const std::string &filepath = "results.json")
	{
		std::lock_guard lock(m_Mutex);
		if (m_CurrentSession)
		{
			// If there is already a current session, then close it before beginning new one.
			// Subsequent profiling output meant for the original session will end up in the
			// newly opened session instead.  That's better than having badly formatted
			// profiling output.
			// 현재 활성화된 세션이 있을 때, 새로운 세션을 시작하기 전에 기존 세션을 종료함으로써 데이터 무결성을
			// 유지하고 혼란을 방지
			if (Log::getCoreLogger()) // Edge case: beginSession() might be before Log::Init()
			{
				AL_CORE_ERROR("Instrumentor::beginSession('{0}') when session '{1}' already open.", name,
							  m_CurrentSession->Name);
			}
			internalEndSession();
		}
		m_OutputStream.open(filepath);

		if (m_OutputStream.is_open())
		{
			m_CurrentSession = new InstrumentationSession({name});
			writeHeader();
		}
		else
		{
			if (Log::getCoreLogger()) // Edge case: beginSession() might be before Log::Init()
			{
				AL_CORE_ERROR("Instrumentor could not open results file '{0}'.", filepath);
			}
		}
	}

	/**
	 * @brief 현재 진행 중인 프로파일링 세션을 종료합니다.
	 */
	void endSession()
	{
		std::lock_guard lock(m_Mutex);
		internalEndSession();
	}

	/**
	 * @brief 프로파일링 데이터를 기록합니다.
	 *
	 * @param result 프로파일링 결과 (`ProfileResult`)
	 */
	void writeProfile(const ProfileResult &result)
	{
		std::stringstream json;

		json << std::setprecision(3) << std::fixed;
		json << ",{";
		json << "\"cat\":\"function\",";
		json << "\"dur\":" << (result.ElapsedTime.count()) << ',';
		json << "\"name\":\"" << result.Name << "\",";
		json << "\"ph\":\"X\",";
		json << "\"pid\":0,";
		json << "\"tid\":" << result.ThreadID << ",";
		json << "\"ts\":" << result.Start.count();
		json << "}";

		std::lock_guard lock(m_Mutex);
		if (m_CurrentSession)
		{
			m_OutputStream << json.str();
			m_OutputStream.flush();
		}
	}

	/**
	 * @brief 싱글톤 인스턴스를 반환합니다.
	 *
	 * @return `Instrumentor`의 전역 인스턴스
	 */
	static Instrumentor &get()
	{
		static Instrumentor instance;
		return instance;
	}

  private:
	Instrumentor() : m_CurrentSession(nullptr)
	{
	}

	~Instrumentor()
	{
		endSession();
	}

	/**
	 * @brief JSON 헤더를 작성합니다.
	 */
	void writeHeader()
	{
		m_OutputStream << "{\"otherData\": {},\"traceEvents\":[{}";
		m_OutputStream.flush();
	}

	/**
	 * @brief JSON 푸터를 작성합니다.
	 */
	void writeFooter()
	{
		m_OutputStream << "]}";
		m_OutputStream.flush();
	}

	/**
	 * @brief 내부적으로 세션을 종료하는 함수 (락을 이미 보유한 상태에서 호출해야 함)
	 */
	void internalEndSession()
	{
		if (m_CurrentSession)
		{
			writeFooter();
			m_OutputStream.close();
			delete m_CurrentSession;
			m_CurrentSession = nullptr;
		}
	}

  private:
	std::mutex m_Mutex;
	InstrumentationSession *m_CurrentSession;
	std::ofstream m_OutputStream;
};

/**
 * @class InstrumentationTimer
 * @brief 코드 실행 시간을 측정하는 타이머 클래스
 *
 * `InstrumentationTimer` 클래스는 특정 코드 블록의 실행 시간을 측정하여 `Instrumentor`에 기록합니다.
 */
class InstrumentationTimer
{
  public:
	/**
	 * @brief 프로파일링 타이머를 생성합니다.
	 *
	 * @param name 프로파일링할 코드 블록의 이름
	 */
	InstrumentationTimer(const char *name) : m_Name(name), m_Stopped(false)
	{
		m_StartTimepoint = std::chrono::steady_clock::now();
	}

	/**
	 * @brief 소멸자에서 자동으로 프로파일링 데이터를 기록합니다.
	 */
	~InstrumentationTimer()
	{
		if (!m_Stopped)
			stop();
	}

	/**
	 * @brief 프로파일링을 종료하고 실행 시간을 기록합니다.
	 */
	void stop()
	{
		auto endTimepoint = std::chrono::steady_clock::now();
		auto highResStart = FloatingPointMicroseconds{m_StartTimepoint.time_since_epoch()};
		auto elapsedTime = std::chrono::time_point_cast<std::chrono::microseconds>(endTimepoint).time_since_epoch() -
						   std::chrono::time_point_cast<std::chrono::microseconds>(m_StartTimepoint).time_since_epoch();

		Instrumentor::get().writeProfile({m_Name, highResStart, elapsedTime, std::this_thread::get_id()});

		m_Stopped = true;
	}

  private:
	const char *m_Name;
	std::chrono::time_point<std::chrono::steady_clock> m_StartTimepoint;
	bool m_Stopped;
};

/**
 * @namespace InstrumentorUtils
 * @brief 문자열 처리를 위한 유틸리티 함수 모음
 */
namespace InstrumentorUtils
{

/**
 * @struct ChangeResult
 * @brief 문자열 변환 결과를 저장하는 구조체
 */
template <size_t N> struct ChangeResult
{
	char Data[N];
};

/**
 * @brief 특정 문자열에서 불필요한 부분을 제거하는 함수
 *
 * @tparam N 원본 문자열 크기
 * @tparam K 제거할 문자열 크기
 * @param expr 원본 문자열
 * @param remove 제거할 문자열
 * @return 변환된 문자열 (`ChangeResult<N>`)
 */
template <size_t N, size_t K> constexpr auto cleanupOutputString(const char (&expr)[N], const char (&remove)[K])
{
	ChangeResult<N> result = {};

	size_t srcIndex = 0;
	size_t dstIndex = 0;
	while (srcIndex < N)
	{
		size_t matchIndex = 0;
		while (matchIndex < K - 1 && srcIndex + matchIndex < N - 1 && expr[srcIndex + matchIndex] == remove[matchIndex])
			matchIndex++;
		if (matchIndex == K - 1)
			srcIndex += matchIndex;
		result.Data[dstIndex++] = expr[srcIndex] == '"' ? '\'' : expr[srcIndex];
		srcIndex++;
	}
	return result;
}
} // namespace InstrumentorUtils
} // namespace ale

/**
 * @defgroup InstrumentorMacros 프로파일링 매크로
 * @brief 성능 분석을 위한 프로파일링 매크로 정의
 * @{
 */
#if AL_PROFILE
#if defined(__GNUC__) || (defined(__MWERKS__) && (__MWERKS__ >= 0x3000)) || (defined(__ICC) && (__ICC >= 600)) ||      \
	defined(__ghs__)
#define HZ_FUNC_SIG __PRETTY_FUNCTION__
#elif defined(__DMC__) && (__DMC__ >= 0x810)
#define HZ_FUNC_SIG __PRETTY_FUNCTION__
#elif (defined(__FUNCSIG__) || (_MSC_VER))
#define HZ_FUNC_SIG __FUNCSIG__
#elif (defined(__INTEL_COMPILER) && (__INTEL_COMPILER >= 600)) || (defined(__IBMCPP__) && (__IBMCPP__ >= 500))
#define HZ_FUNC_SIG __FUNCTION__
#elif defined(__BORLANDC__) && (__BORLANDC__ >= 0x550)
#define HZ_FUNC_SIG __FUNC__
#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901)
#define HZ_FUNC_SIG __func__
#elif defined(__cplusplus) && (__cplusplus >= 201103)
#define HZ_FUNC_SIG __func__
#else
#define HZ_FUNC_SIG "HZ_FUNC_SIG unknown!"
#endif

#define AL_PROFILE_BEGIN_SESSION(name, filepath) ::ale::Instrumentor::get().beginSession(name, filepath)
#define AL_PROFILE_END_SESSION() ::ale::Instrumentor::get().endSession()
#define AL_PROFILE_SCOPE_LINE2(name, line)                                                                             \
	constexpr auto fixedName##line = ::ale::InstrumentorUtils::cleanupOutputString(name, "__cdecl ");                  \
	::ale::InstrumentationTimer timer##line(fixedName##line.Data)
#define AL_PROFILE_SCOPE_LINE(name, line) AL_PROFILE_SCOPE_LINE2(name, line)
#define AL_PROFILE_SCOPE(name) AL_PROFILE_SCOPE_LINE(name, __LINE__)
#define AL_PROFILE_FUNCTION() AL_PROFILE_SCOPE(HZ_FUNC_SIG)
#else
#define AL_PROFILE_BEGIN_SESSION(name, filepath)
#define AL_PROFILE_END_SESSION()
#define AL_PROFILE_SCOPE_LINE2()
#define AL_PROFILE_SCOPE_LINE()
#define AL_PROFILE_SCOPE()
#define AL_PROFILE_FUNCTION()
#endif
