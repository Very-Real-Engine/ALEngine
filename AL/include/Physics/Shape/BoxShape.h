#pragma once

#include "Physics/Shape/Shape.h"

namespace ale
{
/**
 * @file
 * @brief 박스(상자) 형태의 충돌 형태(BoxShape)를 정의하는 클래스.
 */

/**
 * @class BoxShape
 * @brief 박스(상자) 형태의 충돌을 처리하는 클래스.
 * @details BoxShape는 Shape 클래스를 상속받아 충돌 감지를 위한 박스 형태의 기하 정보를 관리합니다.
 */
class BoxShape : public Shape
{
  public:
	/**
	 * @brief BoxShape 기본 생성자.
	 */
	BoxShape();

	/**
	 * @brief BoxShape 기본 소멸자.
	 */
	virtual ~BoxShape() = default;

	/**
	 * @brief BoxShape 객체를 복제(clone)합니다.
	 * @return 복제된 BoxShape 객체의 포인터.
	 */
	BoxShape *clone() const;

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
	 * @brief 박스의 정점(vertices)을 설정합니다.
	 * @param center 박스의 중심 좌표.
	 * @param size 박스의 크기.
	 */
	void setVertices(const alglm::vec3 &center, const alglm::vec3 &size);

	/**
	 * @brief 박스의 ConvexInfo를 반환합니다.
	 * @param transform 적용할 변환 정보.
	 * @return 변환된 ConvexInfo 객체.
	 */
	virtual ConvexInfo getShapeInfo(const Transform &transform) const override;

	// Vertex Info needed
	std::set<alglm::vec3, Vec3Comparator> m_vertices;
	alglm::vec3 m_halfSize;
};
} // namespace ale