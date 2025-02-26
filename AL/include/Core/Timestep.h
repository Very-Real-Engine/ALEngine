#pragma once

/**
 * @file Timestep.h
 * @brief 시간 간격을 표현하는 클래스 정의
 *
 * 이 파일은 애플리케이션의 시간 간격을 관리하는 `Timestep` 클래스를 정의합니다.
 * `std::chrono` 라이브러리를 활용하여 시간 측정을 수행합니다.
 */

#include "Core/Base.h"
#include "Renderer/Common.h"

using namespace std::chrono_literals;

namespace ale
{
/**
 * @namespace Chrono
 * @brief 시간 관련 타입 정의를 포함하는 네임스페이스
 *
 * `std::chrono`의 타입을 별칭으로 정의하여 가독성을 향상시킵니다.
 */
namespace Chrono
{
using TimePoint = std::chrono::time_point<std::chrono::high_resolution_clock>;
using Duration = std::chrono::duration<float, std::chrono::seconds::period>;
} // namespace Chrono

/**
 * @class Timestep
 * @brief 시간 간격을 표현하는 클래스
 *
 * `Timestep` 클래스는 게임 루프 또는 애플리케이션의 시간 간격을 관리하는 데 사용됩니다.
 * 초 단위(`seconds`), 밀리초 단위(`milliseconds`) 변환을 지원하며, 다양한 연산자 오버로딩을 제공합니다.
 */
class Timestep
{
  public:
	/**
	 * @brief 기본 생성자
	 *
	 * 시간 간격을 0으로 초기화합니다.
	 */
	Timestep();

	/**
	 * @brief 특정 시간 값을 가지는 `Timestep` 객체를 생성합니다.
	 *
	 * @param time 시간 값 (`std::chrono::duration<float, std::chrono::seconds::period>`)
	 */
	Timestep(std::chrono::duration<float, std::chrono::seconds::period> time);

	/**
	 * @brief 시간 간격을 초 단위로 반환합니다.
	 *
	 * @return 시간 간격 (`std::chrono::duration<float, std::chrono::seconds::period>`)
	 */
	std::chrono::duration<float, std::chrono::seconds::period> getSeconds() const;

	/**
	 * @brief 시간 간격을 밀리초 단위로 반환합니다.
	 *
	 * @return 시간 간격 (`std::chrono::duration<float, std::chrono::milliseconds::period>`)
	 */
	std::chrono::duration<float, std::chrono::milliseconds::period> getMiliSeconds() const;

	/**
	 * @brief 현재 시간 간격을 출력합니다.
	 */
	void print() const;

	/**
	 * @brief 시간 간격을 부동소수점 값으로 반환합니다.
	 *
	 * @return 시간 간격 (`float`)
	 */
	float count() const;

	/**
	 * @brief `std::chrono::duration` 값을 `Timestep` 객체에 할당합니다.
	 *
	 * @param timestep 할당할 시간 값
	 * @return `Timestep` 객체 참조
	 */
	Timestep &operator=(const std::chrono::duration<float, std::chrono::seconds::period> &timestep);

	/**
	 * @brief `Timestep` 값에서 다른 `Timestep` 값을 뺍니다.
	 *
	 * @param other 뺄셈할 `Timestep` 객체
	 * @return `Timestep` 객체 참조
	 */
	Timestep &operator-=(const Timestep &other);

	/**
	 * @brief 두 `Timestep` 객체 간의 차를 반환합니다.
	 *
	 * @param other 뺄셈할 `Timestep` 객체
	 * @return 차이(`Timestep` 객체)
	 */
	Timestep operator-(const Timestep &other) const;

	/**
	 * @brief `Timestep` 값을 특정 배율로 곱합니다.
	 *
	 * @param factor 곱할 배율 (`float`)
	 * @return 배율이 적용된 `Timestep` 객체
	 */
	Timestep operator*(float factor) const;

	/**
	 * @brief 현재 `Timestep` 값이 다른 `Timestep` 값보다 작거나 같은지 비교합니다.
	 *
	 * @param other 비교할 시간 값
	 * @return `true`이면 작거나 같음, `false`이면 큼
	 */
	bool operator<=(const std::chrono::duration<float, std::chrono::seconds::period> &other) const;

	/**
	 * @brief `Timestep`을 `float` 타입으로 변환합니다.
	 *
	 * @return `Timestep`의 초 단위 부동소수점 값
	 */
	operator float() const
	{
		return m_Timestep.count();
	}

	/**
	 * @brief `alglm::vec3`와 `Timestep` 값을 곱하는 연산자
	 *
	 * @param other 벡터 값
	 * @return 각 요소가 `Timestep` 값과 곱해진 `alglm::vec3`
	 */
	alglm::vec3 operator*(const alglm::vec3 &other) const
	{
		auto ts = m_Timestep.count();
		return alglm::vec3(ts * other.x, ts * other.y, ts * other.z);
	}

  private:
	/**< 시간 간격을 저장하는 변수 */
	std::chrono::duration<float, std::chrono::seconds::period> m_Timestep;
};
} // namespace ale
