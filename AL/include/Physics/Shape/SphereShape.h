#pragma once

#include "Physics/Shape/Shape.h"

struct ConvexInfo;

namespace ale
{
/**
 * @file
 * @brief 구(Sphere) 형태의 충돌 형태(SphereShape)를 정의하는 클래스.
 */

/**
 * @class SphereShape
 * @brief 구(Sphere) 형태의 충돌을 처리하는 클래스.
 * @details SphereShape는 Shape 클래스를 상속받아 충돌 감지를 위한 구 형태의 기하 정보를 관리합니다.
 */
class SphereShape : public Shape
{
  public:
	/**
	 * @brief SphereShape 기본 생성자.
	 */
	SphereShape();

	/**
	 * @brief SphereShape 기본 소멸자.
	 */
	virtual ~SphereShape() = default;

	/**
	 * @brief SphereShape 객체를 복제(clone)합니다.
	 * @return 복제된 SphereShape 객체의 포인터.
	 */
	SphereShape *clone() const;

	/**
	 * @brief 자식(하위) 충돌 개수를 반환합니다.
	 * @return 자식 충돌 개수.
	 */
	int32_t getChildCount() const;

	/**
	 * @brief AABB(Axis-Aligned Bounding Box)를 계산합니다.
	 * @param aabb AABB 정보를 저장할 포인터.
	 * @param xf 변환 정보.
	 */
	void computeAABB(AABB *aabb, const Transform &xf) const;

	/**
	 * @brief 구의 기하학적 특성을 설정합니다.
	 * @param center 구의 중심 좌표.
	 * @param radius 구의 반지름.
	 */
	void setShapeFeatures(const alglm::vec3 &center, float radius);

	/**
	 * @brief 구의 ConvexInfo를 반환합니다.
	 * @param transform 적용할 변환 정보.
	 * @return 변환된 ConvexInfo 객체.
	 */
	virtual ConvexInfo getShapeInfo(const Transform &transform) const override;

	float m_radius;
};
} // namespace ale