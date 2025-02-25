#pragma once

#include "Physics/Shape/Shape.h"

namespace ale
{
class BoxShape : public Shape
{
  public:
	BoxShape();
	virtual ~BoxShape() = default;
	BoxShape *clone() const;
	int32_t getChildCount() const;
	void computeAABB(AABB *aabb, const Transform &xf) const;
	// void setVertices(const std::vector<Vertex> &v);
	void setVertices(const alglm::vec3 &center, const alglm::vec3 &size);
	virtual ConvexInfo getShapeInfo(const Transform &transform) const override;

	// Vertex Info needed
	std::set<alglm::vec3, Vec3Comparator> m_vertices;
	alglm::vec3 m_halfSize;
};
} // namespace ale