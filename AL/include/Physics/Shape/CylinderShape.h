#pragma once

#include "Physics/Shape/Shape.h"

struct ConvexInfo;

namespace ale
{
class CylinderShape : public Shape
{
  public:
	CylinderShape();
	virtual ~CylinderShape() = default;
	CylinderShape *clone() const;
	int32_t getChildCount() const;
	void computeAABB(AABB *aabb, const Transform &xf) const;
	// void setShapeFeatures(const std::vector<Vertex> &vertices);
	// void findAxisByLongestPair(const std::vector<Vertex> &vertices);
	// void computeCylinderRadius(const std::vector<Vertex> &vertices);
	// void computeCylinderFeatures(const std::vector<Vertex> &vertices);
	void setShapeFeatures(const alglm::vec3 &center, float radius, float height);
	void createCylinderPoints();

	virtual ConvexInfo getShapeInfo(const Transform &transform) const override;

	float m_radius;
	float m_height;
	alglm::vec3 m_axes[21];
	alglm::vec3 m_points[40];
	std::set<alglm::vec3, Vec3Comparator> m_vertices;
};
} // namespace ale