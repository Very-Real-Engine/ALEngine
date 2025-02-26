#pragma once

#include "Physics/Contact/Contact.h"

namespace ale
{
/**
 * @file
 * @brief 박스와 실린더 간의 충돌을 처리하는 BoxToCylinderContact 클래스 정의.
 */

/**
 * @class BoxToCylinderContact
 * @brief 박스와 실린더 형태의 물체 간 충돌을 처리하는 클래스.
 * @details BoxToCylinderContact는 Contact 클래스를 상속받아 박스와 실린더 간의 충돌 감지 및 충돌 포인트를 계산하는
 * 역할을 수행합니다.
 */
class BoxToCylinderContact : public Contact
{
  public:
	/**
	 * @brief BoxToCylinderContact 객체를 생성하는 정적 함수.
	 * @param fixtureA 박스의 Fixture.
	 * @param fixtureB 실린더의 Fixture.
	 * @param indexA 박스의 인덱스.
	 * @param indexB 실린더의 인덱스.
	 * @return 생성된 Contact 객체의 포인터.
	 */
	static Contact *create(Fixture *fixtureA, Fixture *fixtureB, int32_t indexA, int32_t indexB);

	/**
	 * @brief BoxToCylinderContact 생성자.
	 * @param fixtureA 박스의 Fixture.
	 * @param fixtureB 실린더의 Fixture.
	 * @param indexA 박스의 인덱스.
	 * @param indexB 실린더의 인덱스.
	 */
	BoxToCylinderContact(Fixture *fixtureA, Fixture *fixtureB, int32_t indexA, int32_t indexB);

	/**
	 * @brief 박스의 Support Function을 계산합니다.
	 * @param box 박스의 ConvexInfo.
	 * @param dir 검색할 방향 벡터.
	 * @return 해당 방향에서의 Support 점 (alglm::vec3).
	 */
	virtual alglm::vec3 supportA(const ConvexInfo &box, alglm::vec3 dir) override;

	/**
	 * @brief 실린더의 Support Function을 계산합니다.
	 * @param cylinder 실린더의 ConvexInfo.
	 * @param dir 검색할 방향 벡터.
	 * @return 해당 방향에서의 Support 점 (alglm::vec3).
	 */
	virtual alglm::vec3 supportB(const ConvexInfo &cylinder, alglm::vec3 dir) override;

	/**
	 * @brief 박스와 실린더 간 충돌 포인트를 계산합니다.
	 * @param box 박스의 ConvexInfo.
	 * @param cylinder 실린더의 ConvexInfo.
	 * @param collisionInfo 충돌 정보를 저장할 구조체.
	 * @param epaInfo EPA(Expanding Polytope Algorithm) 정보를 저장할 구조체.
	 * @param simplexArray 심플렉스 배열.
	 */
	virtual void findCollisionPoints(const ConvexInfo &box, const ConvexInfo &cylinder, CollisionInfo &collisionInfo,
									 EpaInfo &epaInfo, SimplexArray &simplexArray) override;
};
} // namespace ale
