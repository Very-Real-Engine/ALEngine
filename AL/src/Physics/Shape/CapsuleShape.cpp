#include "Physics/Shape/CapsuleShape.h"
#include "Physics/Contact/Contact.h"

namespace ale
{
CapsuleShape::CapsuleShape()
{
	m_type = EType::CAPSULE;
}

CapsuleShape *CapsuleShape::clone() const
{
	void *memory = PhysicsAllocator::m_blockAllocator.allocateBlock(sizeof(CapsuleShape));
	CapsuleShape *clone = new (static_cast<CapsuleShape *>(memory)) CapsuleShape();
	*clone = *this;
	return clone;
}

int32_t CapsuleShape::getChildCount() const
{
	return 1;
}

void CapsuleShape::computeAABB(AABB *aabb, const Transform &xf) const
{
	// update vertices
	std::vector<glm::vec3> vertexVector(m_vertices.begin(), m_vertices.end());
	glm::mat4 rotationMatrix = glm::toMat4(glm::normalize(xf.orientation));
	glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), xf.position);
	glm::mat4 transformMatrix = translationMatrix * rotationMatrix;

	glm::vec3 upper(std::numeric_limits<float>::lowest());
	glm::vec3 lower(std::numeric_limits<float>::max());

	for (glm::vec3 &vertex : vertexVector)
	{
		glm::vec4 v = transformMatrix * glm::vec4(vertex, 1.0f);
		vertex = glm::vec3(v.x, v.y, v.z);

		upper.x = std::max(upper.x, vertex.x);
		upper.y = std::max(upper.y, vertex.y);
		upper.z = std::max(upper.z, vertex.z);
		lower.x = std::min(lower.x, vertex.x);
		lower.y = std::min(lower.y, vertex.y);
		lower.z = std::min(lower.z, vertex.z);
	}

	// 최적화 여지 있음.
	// std::sort(vertexVector.begin(), vertexVector.end(), Vec3Comparator());
	aabb->upperBound = upper + glm::vec3(0.1f);
	aabb->lowerBound = lower - glm::vec3(0.1f);
}

void CapsuleShape::createCapsulePoints()
{
	int32_t segments = 20;
	float angleStep = 2.0f * glm::pi<float>() / static_cast<float>(segments);
	glm::vec3 xAxis(1.0f, 0.0f, 0.0f);

	glm::vec4 topPoint = glm::vec4(m_center + m_height * 0.5f * m_axes[0] + xAxis * m_radius, 1.0f);
	glm::vec4 bottomPoint = glm::vec4(m_center - m_height * 0.5f * m_axes[0] + xAxis * m_radius, 1.0f);

	glm::quat quat = glm::angleAxis(angleStep / 2.0f, m_axes[0]);
	glm::mat4 mat = glm::toMat4(glm::normalize(quat));
	glm::vec3 dir = mat * glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);

	int32_t base = 0;
	for (int32_t i = base; i < segments; i++)
	{
		float theta = i * angleStep;
		glm::quat orientation = glm::angleAxis(theta, m_axes[0]);
		glm::mat4 rotationMatrix = glm::toMat4(glm::normalize(orientation));
		m_points[i] = rotationMatrix * topPoint;
		m_axes[i + 1] = rotationMatrix * glm::vec4(dir, 1.0f);
	}

	base = segments;
	for (int32_t i = base; i < segments + base; i++)
	{
		float theta = (i - base) * angleStep;
		glm::quat orientation = glm::angleAxis(theta, m_axes[0]);
		glm::mat4 rotationMatrix = glm::toMat4(glm::normalize(orientation));
		m_points[i] = rotationMatrix * bottomPoint;
	}
}

void CapsuleShape::setShapeFeatures(const glm::vec3 &center, float radius, float height)
{
	m_center = center;
	m_radius = radius;
	m_height = height;
	createCapsulePoints();
}

ConvexInfo CapsuleShape::getShapeInfo(const Transform &transform) const
{
	glm::mat4 matrix = transform.toMatrix();

	ConvexInfo capsule;
	capsule.radius = m_radius;
	capsule.height = m_height;
	capsule.center = matrix * glm::vec4(m_center, 1.0f);

	int32_t segments = 20;
	int32_t len = segments * 2;

	int32_t axesSize = segments + 1;
	capsule.axesCount = axesSize;
	void *memory = PhysicsAllocator::m_blockAllocator.allocateBlock(sizeof(glm::vec3) * capsule.axesCount);
	capsule.axes = static_cast<glm::vec3 *>(memory);

	capsule.axes[0] = glm::normalize(matrix * glm::vec4(m_axes[0], 0.0f));
	for (int32_t i = 1; i < axesSize; ++i)
	{
		capsule.axes[i] = matrix * glm::vec4(m_axes[i], 1.0f);
	}

	int32_t pointsSize = segments * 2;
	capsule.pointsCount = pointsSize;
	memory = PhysicsAllocator::m_blockAllocator.allocateBlock(sizeof(glm::vec3) * capsule.pointsCount);
	capsule.points = static_cast<glm::vec3 *>(memory);

	for (int32_t i = 0; i < segments; i++)
	{
		capsule.points[i] = matrix * glm::vec4(m_points[i], 1.0f);
		capsule.points[i + segments] = matrix * glm::vec4(m_points[i + segments], 1.0f);
	}

	return capsule;
}
} // namespace ale