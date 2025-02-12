#include "Renderer/Mesh.h"

namespace ale
{
std::shared_ptr<Mesh> Mesh::createMesh(std::vector<Vertex> &vertices, std::vector<uint32_t> &indices)
{
	std::shared_ptr<Mesh> mesh = std::shared_ptr<Mesh>(new Mesh());
	mesh->initMesh(vertices, indices);
	return mesh;
}

std::shared_ptr<Mesh> Mesh::createBox()
{
	std::vector<Vertex> vertices = {
		Vertex{glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(0.0f, 0.0f)},
		Vertex{glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(1.0f, 0.0f)},
		Vertex{glm::vec3(0.5f, 0.5f, -0.5f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(1.0f, 1.0f)},
		Vertex{glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(0.0f, 1.0f)},

		Vertex{glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 0.0f)},
		Vertex{glm::vec3(0.5f, -0.5f, 0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 0.0f)},
		Vertex{glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 1.0f)},
		Vertex{glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 1.0f)},

		Vertex{glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 0.0f)},
		Vertex{glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 1.0f)},
		Vertex{glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 1.0f)},
		Vertex{glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 0.0f)},

		Vertex{glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 0.0f)},
		Vertex{glm::vec3(0.5f, 0.5f, -0.5f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 1.0f)},
		Vertex{glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 1.0f)},
		Vertex{glm::vec3(0.5f, -0.5f, 0.5f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 0.0f)},

		Vertex{glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(0.0f, 1.0f)},
		Vertex{glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(1.0f, 1.0f)},
		Vertex{glm::vec3(0.5f, -0.5f, 0.5f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(1.0f, 0.0f)},
		Vertex{glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(0.0f, 0.0f)},

		Vertex{glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 1.0f)},
		Vertex{glm::vec3(0.5f, 0.5f, -0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(1.0f, 1.0f)},
		Vertex{glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(1.0f, 0.0f)},
		Vertex{glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 0.0f)},
	};

	std::vector<uint32_t> indices = {
		0,	2,	1,	2,	0,	3,	4,	5,	6,	6,	7,	4,	8,	9,	10, 10, 11, 8,
		12, 14, 13, 14, 12, 15, 16, 17, 18, 18, 19, 16, 20, 22, 21, 22, 20, 23,
	};

	return createMesh(vertices, indices);
}

std::shared_ptr<Mesh> Mesh::createSphere()
{
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	uint32_t latiSegmentCount = 16;
	uint32_t longiSegmentCount = 32;

	uint32_t circleVertCount = longiSegmentCount + 1;
	vertices.resize((latiSegmentCount + 1) * circleVertCount);
	for (uint32_t i = 0; i <= latiSegmentCount; i++)
	{
		float v = (float)i / (float)latiSegmentCount;
		float phi = (v - 0.5f) * glm::pi<float>();
		auto cosPhi = cosf(phi);
		auto sinPhi = sinf(phi);
		for (uint32_t j = 0; j <= longiSegmentCount; j++)
		{
			float u = (float)j / (float)longiSegmentCount;
			float theta = u * glm::pi<float>() * 2.0f;
			auto cosTheta = cosf(theta);
			auto sinTheta = sinf(theta);
			auto point = glm::vec3(cosPhi * cosTheta, sinPhi, -cosPhi * sinTheta);

			vertices[i * circleVertCount + j] = Vertex{point * 0.5f, point, glm::vec2(u, v)};
		}
	}

	indices.resize(latiSegmentCount * longiSegmentCount * 6);
	for (uint32_t i = 0; i < latiSegmentCount; i++)
	{
		for (uint32_t j = 0; j < longiSegmentCount; j++)
		{
			uint32_t vertexOffset = i * circleVertCount + j;
			uint32_t indexOffset = (i * longiSegmentCount + j) * 6;
			indices[indexOffset] = vertexOffset;
			indices[indexOffset + 1] = vertexOffset + 1;
			indices[indexOffset + 2] = vertexOffset + 1 + circleVertCount;
			indices[indexOffset + 3] = vertexOffset;
			indices[indexOffset + 4] = vertexOffset + 1 + circleVertCount;
			indices[indexOffset + 5] = vertexOffset + circleVertCount;
		}
	}
	return createMesh(vertices, indices);
}

std::shared_ptr<Mesh> Mesh::createPlane()
{
	std::vector<Vertex> vertices = {
		Vertex{glm::vec3(-0.5f, -0.5f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 0.0f)},
		Vertex{glm::vec3(0.5f, -0.5f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 0.0f)},
		Vertex{glm::vec3(0.5f, 0.5f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 1.0f)},
		Vertex{glm::vec3(-0.5f, 0.5f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 1.0f)},
	};

	std::vector<uint32_t> indices = {
		0, 1, 2, 2, 3, 0,
	};

	return createMesh(vertices, indices);
}

std::shared_ptr<Mesh> Mesh::createGround()
{
	std::vector<Vertex> vertices = {
		Vertex{glm::vec3(-0.5f, -0.5f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 0.0f)},
		Vertex{glm::vec3(0.5f, -0.5f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(5.0f, 0.0f)},
		Vertex{glm::vec3(0.5f, 0.5f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(5.0f, 5.0f)},
		Vertex{glm::vec3(-0.5f, 0.5f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 5.0f)},
	};

	std::vector<uint32_t> indices = {
		0, 1, 2, 2, 3, 0,
	};

	return createMesh(vertices, indices);
}

void Mesh::cleanup()
{
	m_vertexBuffer->cleanup();
	m_indexBuffer->cleanup();
}

void Mesh::draw(VkCommandBuffer commandBuffer)
{
	m_vertexBuffer->bind(commandBuffer);
	m_indexBuffer->bind(commandBuffer);
	vkCmdDrawIndexed(commandBuffer, m_indexBuffer->getIndexCount(), 1, 0, 0, 0);
}

void Mesh::initMesh(std::vector<Vertex> &vertices, std::vector<uint32_t> &indices)
{
	calculateTangents(vertices, indices);
	calculateAABB(vertices);

	m_vertexBuffer = VertexBuffer::createVertexBuffer(vertices);
	m_indexBuffer = IndexBuffer::createIndexBuffer(indices);
}

void Mesh::calculateTangents(std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices)
{
	for (size_t i = 0; i < indices.size(); i += 3)
	{
		Vertex &v0 = vertices[indices[i]];
		Vertex &v1 = vertices[indices[i + 1]];
		Vertex &v2 = vertices[indices[i + 2]];

		glm::vec3 edge1 = v1.pos - v0.pos;
		glm::vec3 edge2 = v2.pos - v0.pos;

		glm::vec2 deltaUV1 = v1.texCoord - v0.texCoord;
		glm::vec2 deltaUV2 = v2.texCoord - v0.texCoord;

		float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
		if (std::isnan(f))
		{
			f = 0.0f;
		}

		glm::vec3 tangent;
		tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
		tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
		tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

		tangent = glm::normalize(tangent);

		v0.tangent += tangent;
		v1.tangent += tangent;
		v2.tangent += tangent;
	}

	// 정점 Tangent 정규화
	for (auto &vertex : vertices)
	{
		vertex.tangent = glm::normalize(vertex.tangent);
	}
}

void Mesh::calculateAABB(std::vector<Vertex> &vertices)
{
	m_minPos = glm::vec3(FLT_MAX);
	m_maxPos = glm::vec3(-FLT_MAX);

	for (Vertex &vertex : vertices)
	{
		m_minPos = glm::min(m_minPos, vertex.pos);
		m_maxPos = glm::max(m_maxPos, vertex.pos);
	}
}

glm::vec3 Mesh::getMaxPos()
{
	return m_maxPos;
}

glm::vec3 Mesh::getMinPos()
{
	return m_minPos;
}

} // namespace ale