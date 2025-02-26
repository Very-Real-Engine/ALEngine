#pragma once

#include "Physics/Physics.h"

namespace ale
{
/**
 * @file
 * @brief 충돌 감지를 위한 AABB, Transform, Manifold 등의 구조체 정의.
 */

/**
 * @struct AABB
 * @brief Axis-Aligned Bounding Box(축 정렬 바운딩 박스)를 정의하는 구조체.
 * @details AABB는 물체의 충돌 영역을 단순화하여 충돌 감지 연산을 최적화하는 데 사용됩니다.
 */
struct AABB
{
	/**
	 * @brief AABB의 유효성을 검사합니다.
	 * @return AABB가 유효하면 true, 그렇지 않으면 false.
	 */
	bool isValid() const;

	/**
	 * @brief AABB의 중심 좌표를 반환합니다.
	 * @return AABB의 중심 좌표.
	 */
	alglm::vec3 getCenter() const
	{
	}

	/**
	 * @brief AABB의 반지름(Extents)을 반환합니다.
	 * @return AABB의 반지름 벡터.
	 */
	alglm::vec3 getExtents() const
	{
	}

	/**
	 * @brief AABB의 표면적을 계산합니다.
	 * @return AABB의 표면적.
	 */
	float getSurface() const
	{
		alglm::vec3 v = upperBound - lowerBound;

		return (v.x * v.y + v.y * v.z + v.z * v.x) * 2.0f;
	}

	/**
	 * @brief 현재 AABB를 다른 AABB와 결합하여 확장합니다.
	 * @param aabb 추가할 AABB.
	 */
	void combine(const AABB &aabb)
	{
		lowerBound = min(lowerBound, aabb.lowerBound);
		upperBound = max(upperBound, aabb.upperBound);
	}

	/**
	 * @brief 두 개의 AABB를 결합하여 새로운 AABB를 만듭니다.
	 * @param aabb1 첫 번째 AABB.
	 * @param aabb2 두 번째 AABB.
	 */
	void combine(const AABB &aabb1, const AABB &aabb2)
	{
		lowerBound = min(aabb1.lowerBound, aabb2.lowerBound);
		upperBound = max(aabb1.upperBound, aabb2.upperBound);
	}

	/**
	 * @brief 현재 AABB가 특정 AABB를 포함하는지 검사합니다.
	 * @param aabb 검사할 AABB.
	 * @return 포함하면 true, 그렇지 않으면 false.
	 */
	bool contains(const AABB &aabb) const
	{
		bool result = true;

		result = result && lowerBound.x <= aabb.lowerBound.x;
		result = result && lowerBound.y <= aabb.lowerBound.y;
		result = result && lowerBound.z <= aabb.lowerBound.z;

		result = result && aabb.upperBound.x <= upperBound.x;
		result = result && aabb.upperBound.y <= upperBound.y;
		result = result && aabb.upperBound.z <= upperBound.z;

		return result;
	}

	alglm::vec3 lowerBound;
	alglm::vec3 upperBound;
};

/**
 * @struct Transform
 * @brief 물체의 위치(Position)와 회전(Rotation)을 나타내는 구조체.
 */
struct Transform
{
	/**
	 * @brief 기본 생성자.
	 */
	Transform()
	{
	}

	/**
	 * @brief 위치와 회전을 설정하는 생성자.
	 * @param p 위치 벡터.
	 * @param r 회전 쿼터니언.
	 */
	Transform(const alglm::vec3 &p, const alglm::quat &r) : position(p), orientation(r)
	{
	}

	/**
	 * @brief 위치와 회전을 설정합니다.
	 * @param p 위치 벡터.
	 * @param angle 회전 각도.
	 */
	void set(const alglm::vec3 &p, float angle)
	{
		position = p;
		// set orientation by angle
	}

	/**
	 * @brief Transform을 4x4 변환 행렬로 변환합니다.
	 * @return 변환 행렬.
	 */
	alglm::mat4 toMatrix() const
	{
		// Create a 4x4 transformation matrix
		alglm::mat4 translationMatrix = alglm::translate(alglm::mat4(1.0f), position); // Translation
		alglm::mat4 rotationMatrix = alglm::toMat4(alglm::normalize(orientation));	   // Rotation

		// Combine translation and rotation
		return translationMatrix * rotationMatrix;
	}

	alglm::vec3 position;
	alglm::quat orientation;
};

/**
 * @struct Sweep
 * @brief 객체의 선형 보간(Interpolation) 상태를 저장하는 구조체.
 */
struct Sweep
{
	alglm::vec3 p;
	alglm::quat q;
	float alpha;
};

/**
 * @brief 두 AABB가 겹치는지 검사합니다.
 * @param a 첫 번째 AABB.
 * @param b 두 번째 AABB.
 * @return 겹치면 true, 그렇지 않으면 false.
 */
inline bool testOverlap(const AABB &a, const AABB &b)
{
	alglm::vec3 d1, d2;

	d1 = b.lowerBound - a.upperBound;
	d2 = a.lowerBound - b.upperBound;

	if (d1.x > 0 || d1.y > 0 || d1.z > 0)
		return false;
	if (d2.x > 0 || d2.y > 0 || d2.z > 0)
		return false;
	return true;
}

/**
 * @struct ManifoldPoint
 * @brief 충돌 지점(Contact Point)의 정보를 저장하는 구조체.
 */
struct ManifoldPoint
{
	float normalImpulse;  // 법선 방향 충격량
	float tangentImpulse; // 접촉면 충격량
	float seperation;	  // 관통 깊이
	alglm::vec3 pointA;	  // 충돌 지점의 위치
	alglm::vec3 pointB;	  // 충돌 지점의 위치
	alglm::vec3 normal;	  // 법선 벡터
};

const int32_t MAX_MANIFOLD_COUNT = 40;

/**
 * @struct Manifold
 * @brief 충돌 정보를 저장하는 구조체.
 * @details 충돌이 발생한 지점과 개수를 저장하며, 이를 활용하여 충돌 응답을 계산할 수 있습니다.
 */
struct Manifold
{
	ManifoldPoint points[MAX_MANIFOLD_COUNT];
	int32_t pointsCount;
};
} // namespace ale