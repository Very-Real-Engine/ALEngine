#pragma once

/**
 * @file UUID.h
 * @brief UUID(고유 식별자) 클래스를 정의하는 헤더 파일
 *
 * 이 파일은 `UUID` 클래스를 정의하며, 64비트 정수를 기반으로 하는 고유 식별자를 제공합니다.
 * 또한, `std::hash`의 특수화를 포함하여 `UUID`를 `unordered_map`의 키로 사용할 수 있도록 지원합니다.
 */

#include <stdint.h>

namespace ale
{
/**
 * @class UUID
 * @brief 64비트 기반의 고유 식별자(UUID) 클래스
 *
 * `UUID` 클래스는 64비트 정수(`uint64_t`)를 기반으로 고유 식별자를 관리합니다.
 * 주로 객체의 고유한 ID를 생성하고 비교하는 용도로 사용됩니다.
 */
class UUID
{
  public:
	/**
     * @brief 기본 생성자
     *
     * 새로운 랜덤 UUID를 생성합니다.
     */
	UUID();

	/**
     * @brief 지정된 값으로 UUID 객체를 생성합니다.
     *
     * @param uuid 64비트 정수(UUID 값)
     */
	UUID(uint64_t uuid);

	/**
     * @brief 복사 생성자 (기본 제공)
     *
     * @param other 복사할 `UUID` 객체
     */
	UUID(const UUID &other) = default;

	/**
     * @brief UUID를 `uint64_t` 타입으로 변환하는 연산자 오버로딩
     *
     * @return `UUID`의 내부 값 (`uint64_t`)
     */
	operator uint64_t() const
	{
		return m_UUID;
	}

  private:
	/**< 64비트 정수로 저장된 UUID 값 */
	uint64_t m_UUID;
};
} // namespace ale

namespace std
{
/**
 * @struct hash
 * @brief `std::unordered_map`에서 `UUID`를 키로 사용할 수 있도록 해싱을 지원
 *
 * `std::hash`의 특수화를 통해 `UUID`를 `unordered_map`의 키로 사용할 수 있도록 정의합니다.
 */
template <typename T> struct hash;

// template specialization
/**
 * @brief `UUID`에 대한 `std::hash` 특수화
 */
template <> struct hash<ale::UUID>
{
	/**
     * @brief UUID 객체의 해시 값을 반환합니다.
     *
     * @param uuid 해시할 `UUID` 객체
     * @return 해시 값 (`size_t`)
     */
	size_t operator()(const ale::UUID &uuid) const
	{
		return (uint64_t)uuid;
	}
};

} // namespace std