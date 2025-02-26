#pragma once

#include "Physics/Contact/Contact.h"

namespace ale
{
/**
 * @file
 * @brief 구(Sphere)와 박스(Box) 간의 충돌을 처리하는 SphereToBoxContact 클래스 정의.
 */

/**
 * @class SphereToBoxContact
 * @brief 구와 박스 간의 충돌을 처리하는 클래스.
 * @details SphereToBoxContact는 Contact 클래스를 상속받아 구와 박스 간의 충돌 감지 및 충돌 포인트를 계산합니다.
 */
class SphereToBoxContact : public Contact
{
  public:
	/**
	 * @brief SphereToBoxContact 객체를 생성하는 정적 함수.
	 * @param fixtureA 구의 Fixture.
	 * @param fixtureB 박스의 Fixture.
	 * @param indexA 구의 인덱스.
	 * @param indexB 박스의 인덱스.
	 * @return 생성된 Contact 객체의 포인터.
	 */
	static Contact *create(Fixture *fixtureA, Fixture *fixtureB, int32_t indexA, int32_t indexB);

	/**
	 * @brief SphereToBoxContact 생성자.
	 * @param fixtureA 구의 Fixture.
	 * @param fixtureB 박스의 Fixture.
	 * @param indexA 구의 인덱스.
	 * @param indexB 박스의 인덱스.
	 */
	SphereToBoxContact(Fixture *fixtureA, Fixture *fixtureB, int32_t indexA, int32_t indexB);

	/**
	 * @brief 구의 Support Function을 계산합니다.
	 * @param sphere 구의 ConvexInfo.
	 * @param dir 검색할 방향 벡터.
	 * @return 해당 방향에서의 Support 점 (alglm::vec3).
	 */
	virtual alglm::vec3 supportA(const ConvexInfo &sphere, alglm::vec3 dir) override;

	/**
	 * @brief 박스의 Support Function을 계산합니다.
	 * @param box 박스의 ConvexInfo.
	 * @param dir 검색할 방향 벡터.
	 * @return 해당 방향에서의 Support 점 (alglm::vec3).
	 */
	virtual alglm::vec3 supportB(const ConvexInfo &box, alglm::vec3 dir) override;

	/**
	 * @brief 구와 박스 간 충돌 포인트를 계산합니다.
	 * @param sphere 구의 ConvexInfo.
	 * @param box 박스의 ConvexInfo.
	 * @param collisionInfo 충돌 정보를 저장할 구조체.
	 * @param epaInfo EPA(Expanding Polytope Algorithm) 정보를 저장할 구조체.
	 * @param simplexArray 심플렉스 배열.
	 */
	virtual void findCollisionPoints(const ConvexInfo &sphere, const ConvexInfo &box, CollisionInfo &collisionInfo,
									 EpaInfo &epaInfo, SimplexArray &simplexArray) override;
};
} // namespace ale