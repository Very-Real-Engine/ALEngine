#pragma once

#include "Physics/Contact/Contact.h"

namespace ale
{
class SphereToSphereContact : public Contact
{
	/**
	 * @file
	 * @brief 두 개의 구(Sphere) 간 충돌을 처리하는 SphereToSphereContact 클래스 정의.
	 */

	/**
	 * @class SphereToSphereContact
	 * @brief 두 개의 구 간 충돌을 처리하는 클래스.
	 * @details SphereToSphereContact는 Contact 클래스를 상속받아 두 구 사이의 충돌 감지 및 충돌 포인트를 계산합니다.
	 */

  public:
	/**
	 * @brief SphereToSphereContact 객체를 생성하는 정적 함수.
	 * @param fixtureA 첫 번째 구의 Fixture.
	 * @param fixtureB 두 번째 구의 Fixture.
	 * @param indexA 첫 번째 구의 인덱스.
	 * @param indexB 두 번째 구의 인덱스.
	 * @return 생성된 Contact 객체의 포인터.
	 */
	static Contact *create(Fixture *fixtureA, Fixture *fixtureB, int32_t indexA, int32_t indexB);

	/**
	 * @brief SphereToSphereContact 생성자.
	 * @param fixtureA 첫 번째 구의 Fixture.
	 * @param fixtureB 두 번째 구의 Fixture.
	 * @param indexA 첫 번째 구의 인덱스.
	 * @param indexB 두 번째 구의 인덱스.
	 */
	SphereToSphereContact(Fixture *fixtureA, Fixture *fixtureB, int32_t indexA, int32_t indexB);

	/**
	 * @brief 첫 번째 구의 Support Function을 계산합니다.
	 * @param sphere 첫 번째 구의 ConvexInfo.
	 * @param dir 검색할 방향 벡터.
	 * @return 해당 방향에서의 Support 점 (alglm::vec3).
	 */
	virtual alglm::vec3 supportA(const ConvexInfo &sphere, alglm::vec3 dir) override;

	/**
	 * @brief 두 번째 구의 Support Function을 계산합니다.
	 * @param sphere 두 번째 구의 ConvexInfo.
	 * @param dir 검색할 방향 벡터.
	 * @return 해당 방향에서의 Support 점 (alglm::vec3).
	 */
	virtual alglm::vec3 supportB(const ConvexInfo &sphere, alglm::vec3 dir) override;

	/**
	 * @brief 두 개의 구 간 충돌 포인트를 계산합니다.
	 * @param sphereA 첫 번째 구의 ConvexInfo.
	 * @param sphereB 두 번째 구의 ConvexInfo.
	 * @param collisionInfo 충돌 정보를 저장할 구조체.
	 * @param epaInfo EPA(Expanding Polytope Algorithm) 정보를 저장할 구조체.
	 * @param simplexArray 심플렉스 배열.
	 */
	virtual void findCollisionPoints(const ConvexInfo &sphereA, const ConvexInfo &sphereB, CollisionInfo &collisionInfo,
									 EpaInfo &epaInfo, SimplexArray &simplexArray) override;
};
} // namespace ale