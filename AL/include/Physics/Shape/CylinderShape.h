#pragma once

#include "Physics/Shape/Shape.h"

struct ConvexInfo;

namespace ale
{
/**
 * @file
 * @brief 실린더(Cylinder) 형태의 충돌 형태(CylinderShape)를 정의하는 클래스.
 */

/**
 * @class CylinderShape
 * @brief 실린더 형태의 충돌을 처리하는 클래스.
 * @details CylinderShape는 Shape 클래스를 상속받아 충돌 감지를 위한 실린더 형태의 기하 정보를 관리합니다.
 */
class CylinderShape : public Shape
{
  public:
	/**
	 * @brief CylinderShape 기본 생성자.
	 */
	CylinderShape();

	/**
	 * @brief CylinderShape 기본 소멸자자.
	 */
	virtual ~CylinderShape() = default;

	/**
	 * @brief CylinderShape 객체를 복제(clone)합니다.
	 * @return 복제된 CylinderShape 객체의 포인터.
	 */
	CylinderShape *clone() const;

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
	 * @brief 실린더의 기하학적 특성을 설정합니다.
	 * @param center 실린더의 중심 좌표.
	 * @param radius 실린더의 반지름.
	 * @param height 실린더의 높이.
	 */
	void setShapeFeatures(const alglm::vec3 &center, float radius, float height);

	/**
	 * @brief 실린더의 주요 포인트들을 생성합니다.
	 */
	void createCylinderPoints();

	/**
	 * @brief 실린더의 ConvexInfo를 반환합니다.
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