#pragma once

#include "Physics/Physics.h"

namespace ale
{
struct AABB
{
	bool isValid() const;

	alglm::vec3 getCenter() const
	{
	}

	alglm::vec3 getExtents() const
	{
	}

	float getSurface() const
	{
		alglm::vec3 v = upperBound - lowerBound;

		return (v.x * v.y + v.y * v.z + v.z * v.x) * 2.0f;
	}

	void combine(const AABB &aabb)
	{
		lowerBound = min(lowerBound, aabb.lowerBound);
		upperBound = max(upperBound, aabb.upperBound);
	}

	void combine(const AABB &aabb1, const AABB &aabb2)
	{
		lowerBound = min(aabb1.lowerBound, aabb2.lowerBound);
		upperBound = max(aabb1.upperBound, aabb2.upperBound);
	}

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

// translation, rotation
struct Transform
{
	Transform()
	{
	}

	Transform(const alglm::vec3 &p, const alglm::quat &r) : position(p), orientation(r)
	{
	}

	void set(const alglm::vec3 &p, float angle)
	{
		position = p;
		// set orientation by angle
	}

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

struct Sweep
{
	alglm::vec3 p;
	alglm::quat q;
	float alpha;
};

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

struct Manifold
{
	ManifoldPoint points[MAX_MANIFOLD_COUNT];
	int32_t pointsCount;
};
} // namespace ale