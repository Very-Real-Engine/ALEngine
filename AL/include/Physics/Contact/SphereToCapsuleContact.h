#pragma once

#include "Physics/Contact/Contact.h"

namespace ale
{
/**
 * @file
 * @brief 구(Sphere)와 캡슐(Capsule) 간의 충돌을 처리하는 SphereToCapsuleContact 클래스 정의.
 */

/**
 * @class SphereToCapsuleContact
 * @brief 구와 캡슐 간의 충돌을 처리하는 클래스.
 * @details SphereToCapsuleContact는 Contact 클래스를 상속받아 구와 캡슐 간의 충돌 감지 및 충돌 포인트를 계산합니다.
 */
class SphereToCapsuleContact : public Contact
{
  public:
	/**
	 * @brief SphereToCapsuleContact 객체를 생성하는 정적 함수.
	 * @param fixtureA 구의 Fixture.
	 * @param fixtureB 캡슐의 Fixture.
	 * @param indexA 구의 인덱스.
	 * @param indexB 캡슐의 인덱스.
	 * @return 생성된 Contact 객체의 포인터.
	 */
	static Contact *create(Fixture *fixtureA, Fixture *fixtureB, int32_t indexA, int32_t indexB);

	/**
	 * @brief SphereToCapsuleContact 생성자.
	 * @param fixtureA 구의 Fixture.
	 * @param fixtureB 캡슐의 Fixture.
	 * @param indexA 구의 인덱스.
	 * @param indexB 캡슐의 인덱스.
	 */
	SphereToCapsuleContact(Fixture *fixtureA, Fixture *fixtureB, int32_t indexA, int32_t indexB);

	/**
	 * @brief 구의 Support Function을 계산합니다.
	 * @param sphere 구의 ConvexInfo.
	 * @param dir 검색할 방향 벡터.
	 * @return 해당 방향에서의 Support 점 (alglm::vec3).
	 */
	virtual alglm::vec3 supportA(const ConvexInfo &sphere, alglm::vec3 dir) override;

	/**
	 * @brief 캡슐의 Support Function을 계산합니다.
	 * @param capsule 캡슐의 ConvexInfo.
	 * @param dir 검색할 방향 벡터.
	 * @return 해당 방향에서의 Support 점 (alglm::vec3).
	 */
	virtual alglm::vec3 supportB(const ConvexInfo &capsule, alglm::vec3 dir) override;

	/**
	 * @brief 구와 캡슐 간 충돌 포인트를 계산합니다.
	 * @param sphere 구의 ConvexInfo.
	 * @param capsule 캡슐의 ConvexInfo.
	 * @param collisionInfo 충돌 정보를 저장할 구조체.
	 * @param epaInfo EPA(Expanding Polytope Algorithm) 정보를 저장할 구조체.
	 * @param simplexArray 심플렉스 배열.
	 */
	virtual void findCollisionPoints(const ConvexInfo &sphere, const ConvexInfo &capsule, CollisionInfo &collisionInfo,
									 EpaInfo &epaInfo, SimplexArray &simplexArray) override;
};
} // namespace ale