#pragma once

#include "Physics/Contact/ContactManager.h"
#include "Physics/Island.h"

#include <stack>

class Model;
class App;

namespace ale
{
class Rigidbody;
class ContactManager;
class BoxShape;
class SphereShape;

/**
 * @file
 * @brief 물리 시뮬레이션을 관리하는 World 클래스 정의.
 */

/**
 * @class World
 * @brief 물리 시뮬레이션을 관리하는 클래스.
 * @details World 클래스는 Rigidbody 및 ContactManager를 관리하며, 물리 연산을 수행합니다.
 */
class World
{
  public:
	/**
	 * @brief World 기본 생성자.
	 */
	World();

	/**
	 * @brief World 소멸자.
	 */
	~World();

	/**
	 * @brief 프레임을 시작할 때 초기화를 수행합니다.
	 */
	void startFrame();

	/**
	 * @brief 물리 연산을 실행합니다.
	 * @param duration 시간 간격.
	 */
	void runPhysics(float duration);

	/**
	 * @brief 충돌 및 물리 해석을 해결합니다.
	 * @param duration 시간 간격.
	 */
	void solve(float duration);

	/**
	 * @brief 특정 Rigidbody에 외력을 등록합니다.
	 * @param idx 적용할 Rigidbody의 인덱스.
	 * @param force 적용할 외력 벡터.
	 */
	void registerBodyForce(int32_t idx, const alglm::vec3 &force);

	/**
	 * @brief 새로운 Rigidbody를 생성합니다.
	 * @param bdDef 생성할 Rigidbody의 설정 정보.
	 * @return 생성된 Rigidbody 포인터.
	 */
	Rigidbody *createBody(BodyDef &bdDef);

	/**
	 * @brief 현재 World에 존재하는 Rigidbody 리스트의 시작 포인터를 반환합니다.
	 * @return Rigidbody 리스트의 시작 포인터.
	 */
	Rigidbody *getBodyList()
	{
		return m_rigidbodies;
	}

	ContactManager m_contactManager;

  private:
	Rigidbody *m_rigidbodies;
	int32_t m_rigidbodyCount;
};
} // namespace ale