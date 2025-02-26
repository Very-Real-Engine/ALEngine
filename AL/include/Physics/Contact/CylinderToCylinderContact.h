#pragma once

#include "Physics/Contact/Contact.h"

namespace ale
{
/**
 * @file
 * @brief 실린더와 실린더 간 충돌을 처리하는 CylinderToCylinderContact 클래스 정의.
 */

/**
 * @class CylinderToCylinderContact
 * @brief 두 개의 실린더 간 충돌을 처리하는 클래스.
 * @details CylinderToCylinderContact는 Contact 클래스를 상속받아 두 실린더 간의 충돌 감지 및 충돌 포인트를 계산합니다.
 */
class CylinderToCylinderContact : public Contact
{
  public:
  /**
	 * @brief CylinderToCylinderContact 객체를 생성하는 정적 함수.
	 * @param fixtureA 첫 번째 실린더의 Fixture.
	 * @param fixtureB 두 번째 실린더의 Fixture.
	 * @param indexA 첫 번째 실린더의 인덱스.
	 * @param indexB 두 번째 실린더의 인덱스.
	 * @return 생성된 Contact 객체의 포인터.
	 */
	static Contact *create(Fixture *fixtureA, Fixture *fixtureB, int32_t indexA, int32_t indexB);
	
		/**
	 * @brief CylinderToCylinderContact 생성자.
	 * @param fixtureA 첫 번째 실린더의 Fixture.
	 * @param fixtureB 두 번째 실린더의 Fixture.
	 * @param indexA 첫 번째 실린더의 인덱스.
	 * @param indexB 두 번째 실린더의 인덱스.
	 */
	CylinderToCylinderContact(Fixture *fixtureA, Fixture *fixtureB, int32_t indexA, int32_t indexB);

	/**
	 * @brief 첫 번째 실린더의 Support Function을 계산합니다.
	 * @param cylinder 첫 번째 실린더의 ConvexInfo.
	 * @param dir 검색할 방향 벡터.
	 * @return 해당 방향에서의 Support 점 (alglm::vec3).
	 */
	virtual alglm::vec3 supportA(const ConvexInfo &cylinder, alglm::vec3 dir) override;
	
	/**
	 * @brief 두 번째 실린더의 Support Function을 계산합니다.
	 * @param cylinder 두 번째 실린더의 ConvexInfo.
	 * @param dir 검색할 방향 벡터.
	 * @return 해당 방향에서의 Support 점 (alglm::vec3).
	 */
	virtual alglm::vec3 supportB(const ConvexInfo &cylinder, alglm::vec3 dir) override;
	
	/**
	 * @brief 두 실린더 간 충돌 포인트를 계산합니다.
	 * @param cylinderA 첫 번째 실린더의 ConvexInfo.
	 * @param cylinderB 두 번째 실린더의 ConvexInfo.
	 * @param collisionInfo 충돌 정보를 저장할 구조체.
	 * @param epaInfo EPA(Expanding Polytope Algorithm) 정보를 저장할 구조체.
	 * @param simplexArray 심플렉스 배열.
	 */
	virtual void findCollisionPoints(const ConvexInfo &cylinderA, const ConvexInfo &cylinderB,
									 CollisionInfo &collisionInfo, EpaInfo &epaInfo,
									 SimplexArray &simplexArray) override;
};
} // namespace ale