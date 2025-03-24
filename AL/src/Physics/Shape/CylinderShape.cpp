#include "Physics/Shape/CylinderShape.h"
#include "Physics/Contact/Contact.h"

namespace ale
{
CylinderShape::CylinderShape()
{
	m_type = EType::CYLINDER;
}

CylinderShape *CylinderShape::clone() const
{
	void *memory = PhysicsAllocator::m_blockAllocator.allocateBlock(sizeof(CylinderShape));
	CylinderShape *clone = new (static_cast<CylinderShape *>(memory)) CylinderShape();
	*clone = *this;
	return clone;
}

int32_t CylinderShape::getChildCount() const
{
	return 1;
}

void CylinderShape::computeAABB(AABB *aabb, const Transform &xf) const
{
	// update vertices
	std::vector<alglm::vec3> vertexVector(m_vertices.begin(), m_vertices.end());
	alglm::mat4 transformMatrix = xf.toMatrix();

	alglm::vec3 upper(std::numeric_limits<float>::lowest());
	alglm::vec3 lower(std::numeric_limits<float>::max());

	// 최적화 여지 있음.
	for (alglm::vec3 &vertex : vertexVector)
	{
		alglm::vec4 v = transformMatrix * alglm::vec4(vertex, 1.0f);
		vertex = alglm::vec3(v.x, v.y, v.z);

		upper.x = std::max(upper.x, vertex.x);
		upper.y = std::max(upper.y, vertex.y);
		upper.z = std::max(upper.z, vertex.z);
		lower.x = std::min(lower.x, vertex.x);
		lower.y = std::min(lower.y, vertex.y);
		lower.z = std::min(lower.z, vertex.z);
	}

	// std::cout << "upper: " << upper.x << ", " << upper.y << ", " << upper.z << '\n';
	// std::cout << "lower: " << lower.x << ", " << lower.y << ", " << lower.z << '\n';
	aabb->upperBound = upper + alglm::vec3(0.1f);
	aabb->lowerBound = lower - alglm::vec3(0.1f);
}

// void CylinderShape::computeCylinderFeatures(const std::vector<Vertex> &vertices)
// {
// 	alglm::vec3 min(FLT_MAX);
// 	alglm::vec3 max(-FLT_MAX);

// 	for (const Vertex &vertex : vertices)
// 	{
// 		max.x = std::max(vertex.position.x, max.x);
// 		max.y = std::max(vertex.position.y, max.y);
// 		max.z = std::max(vertex.position.z, max.z);

// 		min.x = std::min(vertex.position.x, min.x);
// 		min.y = std::min(vertex.position.y, min.y);
// 		min.z = std::min(vertex.position.z, min.z);

// 		m_vertices.insert(vertex.position);
// 	}

// 	m_center = (min + max) / 2.0f;
// 	m_axes[0] = alglm::vec3(0.0f, 1.0f, 0.0f);
// 	m_height = max.y - min.y;

// 	m_radius = 0.0f;
// 	alglm::vec2 center(m_center.x, m_center.z);
// 	for (const Vertex &vertex : vertices)
// 	{
// 		m_radius = std::max(m_radius, alglm::length2(alglm::vec2(vertex.position.x, vertex.position.z) - center));
// 	}
// 	m_radius = std::sqrt(m_radius);
// }

void CylinderShape::createCylinderPoints()
{
	int32_t segments = 20;
	float angleStep = 2.0f * alglm::pi<float>() / static_cast<float>(segments);
	alglm::vec3 xAxis(1.0f, 0.0f, 0.0f);
	m_axes[0] = alglm::vec3(0.0f, 1.0f, 0.0f);

	alglm::vec4 topPoint = alglm::vec4(m_center + m_height * 0.5f * m_axes[0] + xAxis * m_radius, 1.0f);
	alglm::vec4 bottomPoint = alglm::vec4(m_center - m_height * 0.5f * m_axes[0] + xAxis * m_radius, 1.0f);

	alglm::quat quat = alglm::angleAxis(angleStep / 2.0f, m_axes[0]);
	alglm::mat4 mat = alglm::toMat4(alglm::normalize(quat));
	alglm::vec3 dir = mat * alglm::vec4(1.0f, 0.0f, 0.0f, 1.0f);

	int32_t base = 0;
	for (int32_t i = base; i < segments; i++)
	{
		float theta = i * angleStep;
		alglm::quat orientation = alglm::angleAxis(theta, m_axes[0]);
		alglm::mat4 rotationMatrix = alglm::toMat4(alglm::normalize(orientation));
		m_points[i] = rotationMatrix * topPoint;
		m_axes[i + 1] = rotationMatrix * alglm::vec4(dir, 1.0f);
	}

	base = segments;
	for (int32_t i = base; i < segments + base; i++)
	{
		float theta = (i - base) * angleStep;
		alglm::quat orientation = alglm::angleAxis(theta, m_axes[0]);
		alglm::mat4 rotationMatrix = alglm::toMat4(alglm::normalize(orientation));
		m_points[i] = rotationMatrix * bottomPoint;
	}
}

void CylinderShape::setShapeFeatures(const alglm::vec3 &center, float radius, float height)
{
	m_center = center;
	m_radius = radius;
	m_height = height;

	// 박스의 8개 꼭짓점 계산
	m_vertices.insert(center + alglm::vec3(-radius, -height / 2, -radius));
	m_vertices.insert(center + alglm::vec3(radius, -height / 2, -radius));
	m_vertices.insert(center + alglm::vec3(-radius, height / 2, -radius));
	m_vertices.insert(center + alglm::vec3(radius, height / 2, -radius));
	m_vertices.insert(center + alglm::vec3(-radius, -height / 2, radius));
	m_vertices.insert(center + alglm::vec3(radius, -height / 2, radius));
	m_vertices.insert(center + alglm::vec3(-radius, height / 2, radius));
	m_vertices.insert(center + alglm::vec3(radius, height / 2, radius));

	createCylinderPoints();
}

// void CylinderShape::setShapeFeatures(const std::vector<Vertex> &vertices)
// {
// 	computeCylinderFeatures(vertices);
// 	createCylinderPoints();
// 	// findAxisByLongestPair(vertices);
// 	// computeCylinderRadius(vertices);
// }

ConvexInfo CylinderShape::getShapeInfo(const Transform &transform) const
{
	alglm::mat4 matrix = transform.toMatrix();

	ConvexInfo cylinder;
	cylinder.radius = m_radius;
	cylinder.height = m_height;
	cylinder.center = matrix * alglm::vec4(m_center, 1.0f);

	int32_t segments = 20;

	int32_t axesSize = segments + 1;
	cylinder.axesCount = axesSize;
	void *memory = PhysicsAllocator::m_blockAllocator.allocateBlock(sizeof(alglm::vec3) * cylinder.axesCount);
	cylinder.axes = static_cast<alglm::vec3 *>(memory);

	cylinder.axes[0] = alglm::normalize(matrix * alglm::vec4(m_axes[0], 0.0f));
	for (int32_t i = 1; i < axesSize; ++i)
	{
		cylinder.axes[i] = matrix * alglm::vec4(m_axes[i], 1.0f);
	}

	cylinder.pointsCount = segments * 2;
	memory = PhysicsAllocator::m_blockAllocator.allocateBlock(sizeof(alglm::vec3) * cylinder.pointsCount);
	cylinder.points = static_cast<alglm::vec3 *>(memory);

	for (int32_t i = 0; i < segments; i++)
	{
		cylinder.points[i] = matrix * alglm::vec4(m_points[i], 1.0f);
		cylinder.points[i + segments] = matrix * alglm::vec4(m_points[i + segments], 1.0f);
	}

	return cylinder;
}

} // namespace ale