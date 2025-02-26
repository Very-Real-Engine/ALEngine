#pragma once

#include "Physics/Collision.h"

#include <set>
#include <vector>

namespace ale
{
/**
 * @file
 * @brief 충돌 형태(Shape)와 관련된 클래스 및 구조체 정의.
 */

/**
 * @struct Vec3Comparator
 * @brief alglm::vec3를 비교하기 위한 비교 연산자 구조체.
 */
struct Vec3Comparator
{
	/**
	 * @brief 두 개의 벡터를 비교합니다.
	 * @param lhs 비교할 첫 번째 벡터.
	 * @param rhs 비교할 두 번째 벡터.
	 * @return lhs가 rhs보다 작으면 true, 그렇지 않으면 false.
	 */
	bool operator()(const alglm::vec3 &lhs, const alglm::vec3 &rhs) const
	{
		if (lhs.x != rhs.x)
			return lhs.x < rhs.x;
		if (lhs.y != rhs.y)
			return lhs.y < rhs.y;
		return lhs.z < rhs.z;
	}
};

/**
 * @enum EType
 * @brief 다양한 충돌 형태(Shape)의 유형을 정의하는 열거형.
 */
enum class EType
{
	SPHERE = (1 << 0),
	BOX = (1 << 1),
	GROUND = (1 << 2),
	CYLINDER = (1 << 3),
	CAPSULE = (1 << 4),
};

/**
 * @brief EType 열거형에 대한 비트 OR 연산자 오버로딩.
 * @param type1 첫 번째 EType 값.
 * @param type2 두 번째 EType 값.
 * @return OR 연산이 적용된 결과값.
 */
int32_t operator|(EType type1, EType type2);

struct ConvexInfo;

/**
 * @class Shape
 * @brief 모든 충돌 형태(Shape)의 기본 클래스.
 * @details Shape 클래스는 충돌 감지를 위한 기본 인터페이스를 제공하며, 개별 형태(Sphere, Box, Cylinder 등)는 이를
 * 상속받아 구현됩니다.
 */
class Shape
{
  public:
	/**
	 * @brief 소멸자.
	 */
	virtual ~Shape() = default;

	/**
	 * @brief Shape 객체를 복제(clone)합니다.
	 * @return 복제된 Shape 객체의 포인터.
	 */
	virtual Shape *clone() const = 0;

	/**
	 * @brief 자식(하위) 충돌 개수를 반환합니다.
	 * @return 자식 충돌 개수.
	 */
	virtual int32_t getChildCount() const = 0;

	/**
	 * @brief AABB(Axis-Aligned Bounding Box)를 계산합니다.
	 * @param aabb AABB 정보를 저장할 포인터.
	 * @param xf 변환 정보.
	 */
	virtual void computeAABB(AABB *aabb, const Transform &xf) const = 0;

	/**
	 * @brief AABB(Axis-Aligned Bounding Box)를 계산합니다.
	 * @param aabb AABB 정보를 저장할 포인터.
	 * @param xf 변환 정보.
	 */
	virtual ConvexInfo getShapeInfo(const Transform &transform) const = 0;

	/**
	 * @brief Shape의 유형(Type)을 반환합니다.
	 * @return Shape의 유형(EType).
	 */
	EType getType() const
	{
		return m_type;
	}

	/**
	 * @brief Shape의 유형(Type)을 설정합니다.
	 * @param type 설정할 Shape 유형(EType).
	 */
	void setType(EType type)
	{
		m_type = type;
	}

	alglm::vec3 m_center;
	EType m_type;
};

} // namespace ale
