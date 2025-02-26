#pragma once

#include <set>

#include "Physics/Shape/Shape.h"

struct ConvexInfo;

namespace ale
{
/**
 * @file
 * @brief 캡슐(Capsule) 형태의 충돌 형태(CapsuleShape)를 정의하는 클래스.
 */

/**
 * @class CapsuleShape
 * @brief 캡슐 형태의 충돌을 처리하는 클래스.
 * @details CapsuleShape는 Shape 클래스를 상속받아 충돌 감지를 위한 캡슐 형태의 기하 정보를 관리합니다.
 */
class CapsuleShape : public Shape
{
  public:
	/**
	 * @brief CapsuleShape 기본 생성자.
	 */
	CapsuleShape();

	/**
	 * @brief CapsuleShape 기본 소멸자.
	 */
	virtual ~CapsuleShape() = default;

	/**
	 * @brief CapsuleShape 객체를 복제(clone)합니다.
	 * @return 복제된 CapsuleShape 객체의 포인터.
	 */
	CapsuleShape *clone() const;

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
	 * @brief 캡슐의 기하학적 특성을 설정합니다.
	 * @param center 캡슐의 중심 좌표.
	 * @param radius 캡슐의 반지름.
	 * @param height 캡슐의 높이.
	 */
	void setShapeFeatures(const alglm::vec3 &center, float radius, float height);

	/**
	 * @brief 캡슐의 주요 포인트들을 생성합니다.
	 */
	void createCapsulePoints();

	/**
	 * @brief 캡슐의 ConvexInfo를 반환합니다.
	 * @param transform 적용할 변환 정보.
	 * @return 변환된 ConvexInfo 객체.
	 */
	virtual ConvexInfo getShapeInfo(const Transform &transform) const override;

	float m_radius;
	float m_height;
	alglm::vec3 m_axes[21];
	alglm::vec3 m_points[40];
	std::set<alglm::vec3, Vec3Comparator> m_vertices;
};
} // namespace ale