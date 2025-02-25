#include "Physics/Shape/SphereShape.h"
#include "Physics/Contact/Contact.h"

namespace ale
{
SphereShape::SphereShape()
{
	m_type = EType::SPHERE;
}

SphereShape *SphereShape::clone() const
{
	void *memory = PhysicsAllocator::m_blockAllocator.allocateBlock(sizeof(SphereShape));
	SphereShape *clone = new (static_cast<SphereShape *>(memory)) SphereShape();
	*clone = *this;
	return clone;
}

int32_t SphereShape::getChildCount() const
{
	return 1;
}

void SphereShape::computeAABB(AABB *aabb, const Transform &xf) const
{
	// get min, max vertex
	alglm::vec3 upper = xf.position + alglm::vec3(m_radius);
	alglm::vec3 lower = xf.position - alglm::vec3(m_radius);

	aabb->upperBound = upper + alglm::vec3(0.1f);
	aabb->lowerBound = lower - alglm::vec3(0.1f);
}

// void SphereShape::setShapeFeatures(std::vector<Vertex> &vertices)
// {
// 	// welzl 알고리즘 나중에 적용 고려
// 	m_center = alglm::vec3(0.0f);
// 	float distance = 0.0f;

// 	for (const Vertex &vertex : vertices)
// 	{
// 		distance = std::max(vertex.position.x * vertex.position.x + vertex.position.y * vertex.position.y +
// 								vertex.position.z * vertex.position.z,
// 							distance);
// 	}

// 	m_radius = std::sqrt(distance);
// }

void SphereShape::setShapeFeatures(const alglm::vec3 &center, float radius)
{
	m_center = center;
	m_radius = radius;
}

ConvexInfo SphereShape::getShapeInfo(const Transform &transform) const
{
	ConvexInfo sphere;
	sphere.radius = m_radius;
	sphere.center = transform.toMatrix() * alglm::vec4(m_center, 1.0f);
	return sphere;
}
} // namespace ale