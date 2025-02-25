#include "Physics/Shape/BoxShape.h"
#include "Physics/Contact/Contact.h"
#include <limits>

namespace ale
{
BoxShape::BoxShape()
{
	m_type = EType::BOX;
}

BoxShape *BoxShape::clone() const
{
	void *memory = PhysicsAllocator::m_blockAllocator.allocateBlock(sizeof(BoxShape));
	BoxShape *clone = new (static_cast<BoxShape *>(memory)) BoxShape();
	*clone = *this;
	return clone;
}

int32_t BoxShape::getChildCount() const
{
	return 1;
}

void BoxShape::computeAABB(AABB *aabb, const Transform &xf) const
{
	// update vertices
	std::vector<alglm::vec3> vertexVector(m_vertices.begin(), m_vertices.end());
	alglm::mat4 rotationMatrix = alglm::toMat4(alglm::normalize(xf.orientation));
	alglm::mat4 translationMatrix = alglm::translate(alglm::mat4(1.0f), xf.position);
	alglm::mat4 transformMatrix = translationMatrix * rotationMatrix;

	alglm::vec3 upper(std::numeric_limits<float>::lowest());
	alglm::vec3 lower(std::numeric_limits<float>::max());

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

	// 최적화 여지 있음.
	// std::sort(vertexVector.begin(), vertexVector.end(), Vec3Comparator());

	// alglm::vec3 upper = *std::prev(vertexVector.end());
	// alglm::vec3 lower = *vertexVector.begin();

	// std::cout << "upper: " << upper.x << ", " << upper.y << ", " << upper.z << '\n';
	// std::cout << "lower: " << lower.x << ", " << lower.y << ", " << lower.z << '\n';

	aabb->upperBound = upper + alglm::vec3(0.1f);
	aabb->lowerBound = lower - alglm::vec3(0.1f);
}

// void BoxShape::setVertices(const std::vector<Vertex> &vertices)
// {
// 	alglm::vec3 maxPos(std::numeric_limits<float>::lowest());
// 	alglm::vec3 minPos(std::numeric_limits<float>::max());

// 	for (const Vertex &vertex : vertices)
// 	{
// 		maxPos.x = std::max(maxPos.x, vertex.position.x);
// 		maxPos.y = std::max(maxPos.y, vertex.position.y);
// 		maxPos.z = std::max(maxPos.z, vertex.position.z);
// 		minPos.x = std::min(minPos.x, vertex.position.x);
// 		minPos.y = std::min(minPos.y, vertex.position.y);
// 		minPos.z = std::min(minPos.z, vertex.position.z);

// 		m_vertices.insert(vertex.position);
// 	}

// 	m_center = (maxPos + minPos) / 2.0f;
// 	m_halfSize = (maxPos - minPos) / 2.0f;
// }

void BoxShape::setVertices(const alglm::vec3 &center, const alglm::vec3 &size)
{
	alglm::vec3 halfSize = size * 0.5f;

	// 박스의 8개 꼭짓점 계산
	std::vector<alglm::vec3> vertices = {center + alglm::vec3(-halfSize.x, -halfSize.y, -halfSize.z),
										 center + alglm::vec3(halfSize.x, -halfSize.y, -halfSize.z),
										 center + alglm::vec3(-halfSize.x, halfSize.y, -halfSize.z),
										 center + alglm::vec3(halfSize.x, halfSize.y, -halfSize.z),
										 center + alglm::vec3(-halfSize.x, -halfSize.y, halfSize.z),
										 center + alglm::vec3(halfSize.x, -halfSize.y, halfSize.z),
										 center + alglm::vec3(-halfSize.x, halfSize.y, halfSize.z),
										 center + alglm::vec3(halfSize.x, halfSize.y, halfSize.z)};

	// 정점 추가
	for (const auto &vertex : vertices)
	{
		m_vertices.insert(vertex);
	}

	m_center = center;
	m_halfSize = halfSize;
}

ConvexInfo BoxShape::getShapeInfo(const Transform &transform) const
{
	ConvexInfo box;
	alglm::mat4 matrix = transform.toMatrix();

	box.center = matrix * alglm::vec4(m_center, 1.0f);
	box.halfSize = m_halfSize;

	box.pointsCount = 8;
	void *memory;
	memory = PhysicsAllocator::m_blockAllocator.allocateBlock(sizeof(alglm::vec3) * box.pointsCount);
	box.points = static_cast<alglm::vec3 *>(memory);

	box.points[0] = matrix * alglm::vec4(m_center - m_halfSize, 1.0f);
	box.points[1] = matrix * alglm::vec4(m_center + alglm::vec3(m_halfSize.x, -m_halfSize.y, -m_halfSize.z), 1.0f);
	box.points[2] = matrix * alglm::vec4(m_center + alglm::vec3(-m_halfSize.x, m_halfSize.y, -m_halfSize.z), 1.0f);
	box.points[3] = matrix * alglm::vec4(m_center + alglm::vec3(-m_halfSize.x, -m_halfSize.y, m_halfSize.z), 1.0f);
	box.points[4] = matrix * alglm::vec4(m_center + alglm::vec3(m_halfSize.x, m_halfSize.y, -m_halfSize.z), 1.0f);
	box.points[5] = matrix * alglm::vec4(m_center + alglm::vec3(m_halfSize.x, -m_halfSize.y, m_halfSize.z), 1.0f);
	box.points[6] = matrix * alglm::vec4(m_center + alglm::vec3(-m_halfSize.x, m_halfSize.y, m_halfSize.z), 1.0f);
	box.points[7] = matrix * alglm::vec4(m_center + m_halfSize, 1.0f);

	box.axesCount = 3;
	memory = PhysicsAllocator::m_blockAllocator.allocateBlock(sizeof(alglm::vec3) * box.axesCount);
	box.axes = static_cast<alglm::vec3 *>(memory);

	box.axes[0] = alglm::normalize(box.points[1] - box.points[0]);
	box.axes[1] = alglm::normalize(box.points[2] - box.points[0]);
	box.axes[2] = alglm::normalize(box.points[3] - box.points[0]);

	return box;
}

} // namespace ale