#pragma once

#include "Physics/Contact/Contact.h"

namespace ale
{
/**
 * @file
 * @brief 캡슐과 캡슐 간의 충돌을 처리하는 CapsuleToCapsuleContact 클래스 정의.
 */

/**
 * @class CapsuleToCapsuleContact
 * @brief 캡슐 형태의 물체 간 충돌을 처리하는 클래스.
 * @details CapsuleToCapsuleContact는 Contact 클래스를 상속받아 두 캡슐 간의 충돌 감지 및 충돌 포인트를 계산하는 역할을
 * 수행합니다.
 */
class CapsuleToCapsuleContact : public Contact
{
  public:
	/**
	 * @brief CapsuleToCapsuleContact 객체를 생성하는 정적 함수.
	 * @param fixtureA 첫 번째 캡슐의 Fixture.
	 * @param fixtureB 두 번째 캡슐의 Fixture.
	 * @param indexA 첫 번째 캡슐의 인덱스.
	 * @param indexB 두 번째 캡슐의 인덱스.
	 * @return 생성된 Contact 객체의 포인터.
	 */
	static Contact *create(Fixture *fixtureA, Fixture *fixtureB, int32_t indexA, int32_t indexB);

	/**
	 * @brief CapsuleToCapsuleContact 생성자.
	 * @param fixtureA 첫 번째 캡슐의 Fixture.
	 * @param fixtureB 두 번째 캡슐의 Fixture.
	 * @param indexA 첫 번째 캡슐의 인덱스.
	 * @param indexB 두 번째 캡슐의 인덱스.
	 */
	CapsuleToCapsuleContact(Fixture *fixtureA, Fixture *fixtureB, int32_t indexA, int32_t indexB);

	/**
	 * @brief 첫 번째 캡슐의 Support Function을 계산합니다.
	 * @param capsule 첫 번째 캡슐의 ConvexInfo.
	 * @param dir 검색할 방향 벡터.
	 * @return 해당 방향에서의 Support 점 (alglm::vec3).
	 */
	virtual alglm::vec3 supportA(const ConvexInfo &capsule, alglm::vec3 dir) override;

	/**
	 * @brief 두 번째 캡슐의 Support Function을 계산합니다.
	 * @param capsule 두 번째 캡슐의 ConvexInfo.
	 * @param dir 검색할 방향 벡터.
	 * @return 해당 방향에서의 Support 점 (alglm::vec3).
	 */
	virtual alglm::vec3 supportB(const ConvexInfo &capsule, alglm::vec3 dir) override;

	/**
	 * @brief 두 캡슐 간 충돌 포인트를 계산합니다.
	 * @param capsuleA 첫 번째 캡슐의 ConvexInfo.
	 * @param capsuleB 두 번째 캡슐의 ConvexInfo.
	 * @param collisionInfo 충돌 정보를 저장할 구조체.
	 * @param epaInfo EPA(Expanding Polytope Algorithm) 정보를 저장할 구조체.
	 * @param simplexArray 심플렉스 배열.
	 */
	virtual void findCollisionPoints(const ConvexInfo &capsuleA, const ConvexInfo &capsuleB,
									 CollisionInfo &collisionInfo, EpaInfo &epaInfo,
									 SimplexArray &simplexArray) override;
};
} // namespace ale