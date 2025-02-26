#pragma once

#include "Physics/Island.h"
#include "Physics/PhysicsAllocator.h"

namespace ale
{
/**
 * @file
 * @brief 충돌 해결(Contact Solving)과 관련된 구조체 및 ContactSolver 클래스 정의.
 */

/**
 * @struct ContactPositionConstraint
 * @brief 충돌 위치(Position) 제약 조건을 저장하는 구조체.
 */
struct ContactPositionConstraint
{
	ManifoldPoint *points;
	int32_t pointCount;
	alglm::vec3 worldCenterA;
	alglm::vec3 worldCenterB;
	alglm::mat3 invIA, invIB;
	int32_t indexA;
	int32_t indexB;
	float invMassA, invMassB;

	/** @brief 기본 소멸자. */
	~ContactPositionConstraint() = default;
};

/**
 * @struct ContactVelocityConstraint
 * @brief 충돌 속도(Velocity) 제약 조건을 저장하는 구조체.
 */
struct ContactVelocityConstraint
{
	ManifoldPoint *points;
	int32_t pointCount;
	alglm::vec3 worldCenterA;
	alglm::vec3 worldCenterB;
	alglm::mat3 invIA, invIB;
	int32_t indexA, indexB;
	float invMassA, invMassB;
	float friction;
	float restitution;

	/** @brief 기본 소멸자. */
	~ContactVelocityConstraint() = default;
};

/**
 * @class ContactSolver
 * @brief 물리 엔진에서 충돌을 해결하는 클래스.
 * @details ContactSolver는 위치(Position) 및 속도(Velocity) 제약 조건을 기반으로 충돌을 해결하고, 객체 간의 역학적
 * 반응을 계산합니다.
 */
class ContactSolver
{
  public:
	/**
	 * @brief ContactSolver 생성자.
	 * @param duration 시뮬레이션 시간 간격.
	 * @param contacts 충돌(Contact) 객체 배열.
	 * @param positions 개체들의 위치(Position) 배열.
	 * @param velocities 개체들의 속도(Velocity) 배열.
	 * @param bodyCount 물리 연산을 수행할 개체 개수.
	 * @param contactCount 충돌 개수.
	 */
	ContactSolver(float duration, Contact **contacts, Position *positions, Velocity *velocities, int32_t bodyCount,
				  int32_t contactCount);

	/**
	 * @brief ContactSolver 객체를 해제합니다.
	 */
	void destroy();

	/**
	 * @brief 속도 제약 조건을 초기화합니다.
	 */
	void initializeVelocityConstraints();

	/**
	 * @brief 속도 제약 조건을 해결합니다.
	 */
	void solveVelocityConstraints();

	/**
	 * @brief 위치 제약 조건을 해결합니다.
	 */
	void solvePositionConstraints();

	/**
	 * @brief 충돌 객체가 정지 상태(Sleep)에 도달했는지 확인합니다.
	 */
	void checkSleepContact();

	static const float NORMAL_STOP_VELOCITY;
	static const float TANGENT_STOP_VELOCITY;
	static const float NORMAL_SLEEP_VELOCITY;
	static const float TANGENT_SLEEP_VELOCITY;

	int32_t m_bodyCount;
	int32_t m_contactCount;
	float m_duration;
	Contact **m_contacts;
	Position *m_positions;
	Velocity *m_velocities;
	ContactPositionConstraint *m_positionConstraints;
	ContactVelocityConstraint *m_velocityConstraints;
};

} // namespace ale