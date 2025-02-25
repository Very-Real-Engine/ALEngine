#pragma once

#include <set>

#include "Physics/Shape/Shape.h"

struct ConvexInfo;

namespace ale
{
class CapsuleShape : public Shape
{
  public:
	CapsuleShape();
	virtual ~CapsuleShape() = default;
	CapsuleShape *clone() const;
	int32_t getChildCount() const;
	void computeAABB(AABB *aabb, const Transform &xf) const;
	// void setShapeFeatures(const std::vector<Vertex> &vertices);
	// void computeCapsuleFeatures(const std::vector<Vertex> &vertices);
	void setShapeFeatures(const alglm::vec3 &center, float radius, float height);
	void createCapsulePoints();
	virtual ConvexInfo getShapeInfo(const Transform &transform) const override;

	float m_radius;
	float m_height;
	alglm::vec3 m_axes[21];
	alglm::vec3 m_points[40];
	std::set<alglm::vec3, Vec3Comparator> m_vertices;
};
} // namespace ale