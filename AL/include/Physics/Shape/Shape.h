#pragma once

#include "Physics/Collision.h"

#include <set>
#include <vector>

namespace ale
{
struct Vec3Comparator
{
	bool operator()(const alglm::vec3 &lhs, const alglm::vec3 &rhs) const
	{
		if (lhs.x != rhs.x)
			return lhs.x < rhs.x;
		if (lhs.y != rhs.y)
			return lhs.y < rhs.y;
		return lhs.z < rhs.z;
	}
};

enum class EType
{
	SPHERE = (1 << 0),
	BOX = (1 << 1),
	GROUND = (1 << 2),
	CYLINDER = (1 << 3),
	CAPSULE = (1 << 4),
};

int32_t operator|(EType type1, EType type2);

struct ConvexInfo;

class Shape
{
  public:
	virtual ~Shape() = default;
	virtual Shape *clone() const = 0;
	virtual int32_t getChildCount() const = 0;
	virtual void computeAABB(AABB *aabb, const Transform &xf) const = 0;
	virtual ConvexInfo getShapeInfo(const Transform &transform) const = 0;

	EType getType() const
	{
		return m_type;
	}
	void setType(EType type)
	{
		m_type = type;
	}

	alglm::vec3 m_center;
	EType m_type;
};

} // namespace ale
