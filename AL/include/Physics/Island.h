#pragma once

#include "Physics/Contact/Contact.h"

namespace ale
{
/**
 * @file
 * @brief 물리 시뮬레이션의 연산 단위인 Island 클래스 및 관련 구조체 정의.
 */

/**
 * @struct Position
 * @brief 물리 객체의 위치 정보를 저장하는 구조체.
 */
struct Position
{
	alglm::vec3 position;
	alglm::vec3 positionBuffer;
	bool isNormalStop{true};
	bool isTangentStop{true};
	bool isNormal{false};
};

/**
 * @struct Velocity
 * @brief 물리 객체의 속도 정보를 저장하는 구조체.
 */
struct Velocity
{
	alglm::vec3 linearVelocity;
	alglm::vec3 angularVelocity;
	alglm::vec3 linearVelocityBuffer;
	alglm::vec3 angularVelocityBuffer;
};

/**
 * @class Island
 * @brief 물리 연산을 수행하는 독립적인 객체 그룹을 관리하는 클래스.
 * @details Island는 충돌이 연관된 Rigidbody들을 그룹화하여 해결하는 단위로 사용됩니다.
 */
class Island
{
  public:
	/**
	 * @brief Island 생성자.
	 * @param bodyCount 포함할 Rigidbody 개수.
	 * @param contactCount 포함할 Contact 개수.
	 */
	Island(int32_t bodyCount, int32_t contactCount);

	/**
	 * @brief Island 내의 물리 연산을 해결합니다.
	 * @param duration 시뮬레이션 시간 간격.
	 */
	void solve(float duration);

	/**
	 * @brief Island를 정리하고 할당된 자원을 해제합니다.
	 */
	void destroy();

	/**
	 * @brief Rigidbody를 Island에 추가합니다.
	 * @param body 추가할 Rigidbody.
	 */
	void add(Rigidbody *body);

	/**
	 * @brief Contact를 Island에 추가합니다.
	 * @param contact 추가할 Contact.
	 */
	void add(Contact *contact);

	/**
	 * @brief Island의 모든 객체를 초기화합니다.
	 */
	void clear();

	static const int32_t VELOCITY_ITERATION;
	static const int32_t POSITION_ITERATION;
	static const float STOP_LINEAR_VELOCITY;
	static const float STOP_ANGULAR_VELOCITY;

	Rigidbody **m_bodies;
	Contact **m_contacts;
	Position *m_positions;
	Velocity *m_velocities;

	int32_t m_bodyCount;
	int32_t m_contactCount;
};

} // namespace ale
