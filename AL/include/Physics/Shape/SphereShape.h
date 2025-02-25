#pragma once

#include "Physics/Shape/Shape.h"

struct ConvexInfo;

namespace ale
{
class SphereShape : public Shape
{
  public:
	SphereShape();
	virtual ~SphereShape() = default;
	SphereShape *clone() const;
	int32_t getChildCount() const;
	void computeAABB(AABB *aabb, const Transform &xf) const;
	// void setShapeFeatures(std::vector<Vertex> &vertices);
	void setShapeFeatures(const alglm::vec3 &center, float radius);
	virtual ConvexInfo getShapeInfo(const Transform &transform) const override;

	float m_radius;
};
} // namespace ale