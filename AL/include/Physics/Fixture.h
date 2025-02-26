#pragma once

#include "Physics/Physics.h"
#include "Physics/PhysicsAllocator.h"
#include "Physics/Rigidbody.h"

namespace ale
{
/**
 * @file
 * @brief 물리 엔진에서 충돌을 처리하는 Fixture 클래스 및 관련 구조체 정의.
 */

class BroadPhase;
class Rigidbody;

/**
 * @struct FixtureDef
 * @brief Fixture(충돌 요소)의 속성을 정의하는 구조체.
 */
struct FixtureDef
{
	/**
	 * @brief 기본 생성자. 기본 값을 초기화합니다.
	 */
	FixtureDef()
	{
		shape = nullptr;
		userData = nullptr;
		friction = 0.0f;
		restitution = 0.0f;
		isSensor = false;
		touchNum = 0;
	}
	Shape *shape;
	void *userData;
	float friction;
	float restitution;
	bool isSensor;
	int32_t touchNum;
};

/**
 * @struct FixtureProxy
 * @brief BroadPhase에서 사용되는 Fixture의 프록시 구조체.
 */
struct FixtureProxy
{
	AABB aabb;
	Fixture *fixture;
	int32_t childIndex;
	int32_t proxyId;
};

/**
 * @class Fixture
 * @brief 물리 엔진에서 충돌을 처리하는 클래스.
 * @details Fixture는 Rigidbody와 Shape을 기반으로 충돌 감지를 수행하며, BroadPhase에서 사용됩니다.
 */
class Fixture
{
  public:
	/**
	 * @brief Fixture 기본 생성자.
	 */
	Fixture();

	/**
	 * @brief Fixture를 생성합니다.
	 * @param body 연결할 Rigidbody.
	 * @param fd Fixture 정의 정보.
	 */
	void create(Rigidbody *body, const FixtureDef *fd);

	/**
	 * @brief Fixture를 제거합니다.
	 */
	void destroy();

	/**
	 * @brief BroadPhase에 프로시(Proxy)를 생성합니다.
	 * @param broadPhase BroadPhase 객체.
	 */
	void createProxies(BroadPhase *broadPhase);

	/**
	 * @brief BroadPhase에서 프로시(Proxy)를 제거합니다.
	 * @param broadPhase BroadPhase 객체.
	 */
	void destroyProxies(BroadPhase *broadPhase);

	/**
	 * @brief BroadPhase에서 AABB를 동기화합니다.
	 * @param broadPhase BroadPhase 객체.
	 * @param xf1 이전 Transform.
	 * @param xf2 현재 Transform.
	 */
	void synchronize(BroadPhase *broadPhase, const Transform &xf1, const Transform &xf2);

	/**
	 * @brief 마찰 계수를 반환합니다.
	 * @return 마찰 계수.
	 */
	float getFriction();

	/**
	 * @brief 반발 계수를 반환합니다.
	 * @return 반발 계수.
	 */
	float getRestitution();

	/**
	 * @brief 연결된 Rigidbody를 반환합니다.
	 * @return Rigidbody 포인터.
	 */
	Rigidbody *getBody() const;

	/**
	 * @brief Fixture의 타입을 반환합니다.
	 * @return EType 값.
	 */
	EType getType() const;

	/**
	 * @brief 연결된 Shape을 반환합니다.
	 * @return Shape 포인터.
	 */
	Shape *getShape();

	/**
	 * @brief 센서 여부를 반환합니다.
	 * @return 센서일 경우 true, 아니면 false.
	 */
	bool isSeonsor() const;

	/**
	 * @brief 현재 터치(충돌) 횟수를 반환합니다.
	 * @return 터치 횟수.
	 */
	int32_t getTouchNum() const;

	/**
	 * @brief 터치(충돌) 횟수를 증가시킵니다.
	 */
	void increaseTouchNum();

	/**
	 * @brief 터치(충돌) 횟수를 감소시킵니다.
	 */
	void decreaseTouchNum();

	/**
	 * @brief Fixture의 프록시 정보를 반환합니다.
	 * @return FixtureProxy 포인터.
	 */
	const FixtureProxy *getFixtureProxy() const;

  protected:
	Rigidbody *m_body;
	Shape *m_shape;
	float m_density;
	float m_friction;
	float m_restitution;
	bool m_isSensor;
	int32_t m_touchNum;

	FixtureProxy *m_proxies;
	int32_t m_proxyCount;
	// void *userData;

  private:
};
} // namespace ale