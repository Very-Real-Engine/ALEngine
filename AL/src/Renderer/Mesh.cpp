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
		Vertex{alglm::vec3(-0.5f, -0.5f, -0.5f), alglm::vec3(0.0f, 0.0f, -1.0f), alglm::vec2(0.0f, 0.0f)},
		Vertex{alglm::vec3(0.5f, -0.5f, -0.5f), alglm::vec3(0.0f, 0.0f, -1.0f), alglm::vec2(1.0f, 0.0f)},
		Vertex{alglm::vec3(0.5f, 0.5f, -0.5f), alglm::vec3(0.0f, 0.0f, -1.0f), alglm::vec2(1.0f, 1.0f)},
		Vertex{alglm::vec3(-0.5f, 0.5f, -0.5f), alglm::vec3(0.0f, 0.0f, -1.0f), alglm::vec2(0.0f, 1.0f)},

		Vertex{alglm::vec3(-0.5f, -0.5f, 0.5f), alglm::vec3(0.0f, 0.0f, 1.0f), alglm::vec2(0.0f, 0.0f)},
		Vertex{alglm::vec3(0.5f, -0.5f, 0.5f), alglm::vec3(0.0f, 0.0f, 1.0f), alglm::vec2(1.0f, 0.0f)},
		Vertex{alglm::vec3(0.5f, 0.5f, 0.5f), alglm::vec3(0.0f, 0.0f, 1.0f), alglm::vec2(1.0f, 1.0f)},
		Vertex{alglm::vec3(-0.5f, 0.5f, 0.5f), alglm::vec3(0.0f, 0.0f, 1.0f), alglm::vec2(0.0f, 1.0f)},

		Vertex{alglm::vec3(-0.5f, 0.5f, 0.5f), alglm::vec3(-1.0f, 0.0f, 0.0f), alglm::vec2(1.0f, 0.0f)},
		Vertex{alglm::vec3(-0.5f, 0.5f, -0.5f), alglm::vec3(-1.0f, 0.0f, 0.0f), alglm::vec2(1.0f, 1.0f)},
		Vertex{alglm::vec3(-0.5f, -0.5f, -0.5f), alglm::vec3(-1.0f, 0.0f, 0.0f), alglm::vec2(0.0f, 1.0f)},
		Vertex{alglm::vec3(-0.5f, -0.5f, 0.5f), alglm::vec3(-1.0f, 0.0f, 0.0f), alglm::vec2(0.0f, 0.0f)},

		Vertex{alglm::vec3(0.5f, 0.5f, 0.5f), alglm::vec3(1.0f, 0.0f, 0.0f), alglm::vec2(1.0f, 0.0f)},
		Vertex{alglm::vec3(0.5f, 0.5f, -0.5f), alglm::vec3(1.0f, 0.0f, 0.0f), alglm::vec2(1.0f, 1.0f)},
		Vertex{alglm::vec3(0.5f, -0.5f, -0.5f), alglm::vec3(1.0f, 0.0f, 0.0f), alglm::vec2(0.0f, 1.0f)},
		Vertex{alglm::vec3(0.5f, -0.5f, 0.5f), alglm::vec3(1.0f, 0.0f, 0.0f), alglm::vec2(0.0f, 0.0f)},

		Vertex{alglm::vec3(-0.5f, -0.5f, -0.5f), alglm::vec3(0.0f, -1.0f, 0.0f), alglm::vec2(0.0f, 1.0f)},
		Vertex{alglm::vec3(0.5f, -0.5f, -0.5f), alglm::vec3(0.0f, -1.0f, 0.0f), alglm::vec2(1.0f, 1.0f)},
		Vertex{alglm::vec3(0.5f, -0.5f, 0.5f), alglm::vec3(0.0f, -1.0f, 0.0f), alglm::vec2(1.0f, 0.0f)},
		Vertex{alglm::vec3(-0.5f, -0.5f, 0.5f), alglm::vec3(0.0f, -1.0f, 0.0f), alglm::vec2(0.0f, 0.0f)},

		Vertex{alglm::vec3(-0.5f, 0.5f, -0.5f), alglm::vec3(0.0f, 1.0f, 0.0f), alglm::vec2(0.0f, 1.0f)},
		Vertex{alglm::vec3(0.5f, 0.5f, -0.5f), alglm::vec3(0.0f, 1.0f, 0.0f), alglm::vec2(1.0f, 1.0f)},
		Vertex{alglm::vec3(0.5f, 0.5f, 0.5f), alglm::vec3(0.0f, 1.0f, 0.0f), alglm::vec2(1.0f, 0.0f)},
		Vertex{alglm::vec3(-0.5f, 0.5f, 0.5f), alglm::vec3(0.0f, 1.0f, 0.0f), alglm::vec2(0.0f, 0.0f)},
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
		float phi = (v - 0.5f) * alglm::pi<float>();
		auto cosPhi = cosf(phi);
		auto sinPhi = sinf(phi);
		for (uint32_t j = 0; j <= longiSegmentCount; j++)
		{
			float u = (float)j / (float)longiSegmentCount;
			float theta = u * alglm::pi<float>() * 2.0f;
			auto cosTheta = cosf(theta);
			auto sinTheta = sinf(theta);
			auto point = alglm::vec3(cosPhi * cosTheta, sinPhi, -cosPhi * sinTheta);

			vertices[i * circleVertCount + j] = Vertex{point * 0.5f, point, alglm::vec2(u, v)};
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
		Vertex{alglm::vec3(-0.5f, -0.5f, 0.0f), alglm::vec3(0.0f, 0.0f, 1.0f), alglm::vec2(0.0f, 0.0f)},
		Vertex{alglm::vec3(0.5f, -0.5f, 0.0f), alglm::vec3(0.0f, 0.0f, 1.0f), alglm::vec2(1.0f, 0.0f)},
		Vertex{alglm::vec3(0.5f, 0.5f, 0.0f), alglm::vec3(0.0f, 0.0f, 1.0f), alglm::vec2(1.0f, 1.0f)},
		Vertex{alglm::vec3(-0.5f, 0.5f, 0.0f), alglm::vec3(0.0f, 0.0f, 1.0f), alglm::vec2(0.0f, 1.0f)},
	};

	std::vector<uint32_t> indices = {
		0, 1, 2, 2, 3, 0,
	};

	return createMesh(vertices, indices);
}

std::shared_ptr<Mesh> Mesh::createGround()
{
	std::vector<Vertex> vertices = {
		Vertex{alglm::vec3(-0.5f, -0.5f, 0.0f), alglm::vec3(0.0f, 0.0f, 1.0f), alglm::vec2(0.0f, 0.0f)},
		Vertex{alglm::vec3(0.5f, -0.5f, 0.0f), alglm::vec3(0.0f, 0.0f, 1.0f), alglm::vec2(5.0f, 0.0f)},
		Vertex{alglm::vec3(0.5f, 0.5f, 0.0f), alglm::vec3(0.0f, 0.0f, 1.0f), alglm::vec2(5.0f, 5.0f)},
		Vertex{alglm::vec3(-0.5f, 0.5f, 0.0f), alglm::vec3(0.0f, 0.0f, 1.0f), alglm::vec2(0.0f, 5.0f)},
	};

	std::vector<uint32_t> indices = {
		0, 1, 2, 2, 3, 0,
	};

	return createMesh(vertices, indices);
}

std::shared_ptr<Mesh> Mesh::createCapsule()
{
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	// 세그먼트 개수 설정
	uint32_t halfLatiSegmentCount = 8;					  // 하단 반구 세로 세그먼트 (equator 포함)
	uint32_t latiSegmentCount = halfLatiSegmentCount * 2; // 전체 세로 세그먼트 (0~16)
	uint32_t longiSegmentCount = 20;					  // 원주 방향 세그먼트
	float radius = 0.5f;

	// 하단 반구: 0 ~ halfLatiSegmentCount (총 halfLatiSegmentCount+1 행)
	// 상단 반구: halfLatiSegmentCount+1 ~ latiSegmentCount (총 latiSegmentCount - halfLatiSegmentCount 행)
	uint32_t bottomRows = halfLatiSegmentCount + 1;				// 9행
	uint32_t topRows = latiSegmentCount - halfLatiSegmentCount; // 8행
	uint32_t totalRows = bottomRows + topRows;					// 17행 전체

	uint32_t circleVertCount = longiSegmentCount + 1; // 한 행에 있는 정점 개수 (마지막 점은 wrap-around)
	vertices.resize(totalRows * circleVertCount);

	// -------- 하단 반구 (바닥 극점 ~ equator) --------
	// moveVectorBottom: 구의 중심을 (0, -radius, 0)로 이동시켜 아래쪽 위치로 배치
	alglm::vec3 moveVectorBottom(0.0f, -radius, 0.0f);
	for (uint32_t i = 0; i < bottomRows; i++)
	{
		// 전역 세로 좌표: i가 0이면 바닥 극점, i가 halfLatiSegmentCount이면 equator
		float v = (float)i / (float)latiSegmentCount; // v ∈ [0, 0.5]
		// 구의 극각(phi): -pi/2 ~ 0
		float phi = (v - 0.5f) * alglm::pi<float>();
		float cosPhi = cosf(phi);
		float sinPhi = sinf(phi);

		for (uint32_t j = 0; j < circleVertCount; j++)
		{
			float u = (float)j / (float)longiSegmentCount; // u ∈ [0, 1]
			float theta = u * alglm::pi<float>() * 2.0f;
			float cosTheta = cosf(theta);
			float sinTheta = sinf(theta);
			// 구의 표면상 좌표 (노말 방향)
			alglm::vec3 point(cosPhi * cosTheta, sinPhi, -cosPhi * sinTheta);
			uint32_t index = i * circleVertCount + j;
			// 정점 위치: 구의 표면 좌표 * radius에 구의 중심 이동 적용
			vertices[index] = Vertex{point * radius + moveVectorBottom, point, alglm::vec2(u, v)};
		}
	}

	// -------- 상단 반구 (equator ~ 상단 극점) --------
	// moveVectorTop: 구의 중심을 (0, radius, 0)로 이동시켜 위쪽 위치로 배치
	alglm::vec3 moveVectorTop(0.0f, radius, 0.0f);
	// 상단 반구는 equator 정점(하단 반구의 마지막 행)을 공유하므로
	// i를 halfLatiSegmentCount+1부터 시작하여 topRows 행을 생성
	for (uint32_t i = halfLatiSegmentCount + 1; i <= latiSegmentCount; i++)
	{
		float v = (float)i / (float)latiSegmentCount; // v ∈ (0.5, 1]
		// 구의 극각(phi): 0 ~ pi/2
		float phi = (v - 0.5f) * alglm::pi<float>();
		float cosPhi = cosf(phi);
		float sinPhi = sinf(phi);

		// 상단 정점의 저장 행: bottomRows행 다음부터 시작
		uint32_t row = i - halfLatiSegmentCount; // row ∈ [1, topRows]
		for (uint32_t j = 0; j < circleVertCount; j++)
		{
			float u = (float)j / (float)longiSegmentCount;
			float theta = u * alglm::pi<float>() * 2.0f;
			float cosTheta = cosf(theta);
			float sinTheta = sinf(theta);
			alglm::vec3 point(cosPhi * cosTheta, sinPhi, -cosPhi * sinTheta);
			// 전체 행 인덱스: bottomRows(=하단 행 수) + (row-1)
			uint32_t index = (bottomRows + row - 1) * circleVertCount + j;
			vertices[index] = Vertex{point * radius + moveVectorTop, point, alglm::vec2(u, v)};
		}
	}

	// -------- 인덱스 생성 (각 사각형을 두 개의 삼각형으로 분할) --------
	// 전체 행 수: totalRows, 각 행에 circleVertCount개의 정점이 있음.
	// (totalRows - 1) * longiSegmentCount 개의 사각형(quad)이 있음.
	indices.resize((totalRows - 1) * longiSegmentCount * 6);
	for (uint32_t i = 0; i < totalRows - 1; i++)
	{
		for (uint32_t j = 0; j < longiSegmentCount; j++)
		{
			uint32_t vertexOffset = i * circleVertCount + j;
			uint32_t indexOffset = (i * longiSegmentCount + j) * 6;
			indices[indexOffset + 0] = vertexOffset;
			indices[indexOffset + 1] = vertexOffset + 1;
			indices[indexOffset + 2] = vertexOffset + 1 + circleVertCount;
			indices[indexOffset + 3] = vertexOffset;
			indices[indexOffset + 4] = vertexOffset + 1 + circleVertCount;
			indices[indexOffset + 5] = vertexOffset + circleVertCount;
		}
	}

	return createMesh(vertices, indices);
}

std::shared_ptr<Mesh> Mesh::createCylinder()
{
	std::vector<Vertex> vertices;

	int32_t segments = 20.0f;
	float halfHeight = 0.5f;
	float radius = 0.5f;

	float angleStep = 2.0f * alglm::pi<float>() / static_cast<float>(segments);

	// Top cap center
	alglm::vec3 topCenter(0.0f, halfHeight, 0.0f);
	vertices.push_back({topCenter, alglm::vec3(0.0f, 1.0f, 0.0f), alglm::vec2(0.5f, 0.5f)});

	// Top cap vertices
	for (int32_t i = 0; i <= segments; ++i)
	{
		float theta = i * angleStep;
		alglm::vec3 position(radius * cos(theta), halfHeight, radius * sin(theta));
		alglm::vec2 texCoord(0.5f + 0.5f * cos(theta), 0.5f + 0.5f * sin(theta));
		vertices.push_back({position, alglm::vec3(0.0f, 1.0f, 0.0f), texCoord});
	}

	// Bottom cap center
	alglm::vec3 bottomCenter(0.0f, -halfHeight, 0.0f);
	vertices.push_back({bottomCenter, alglm::vec3(0.0f, -1.0f, 0.0f), alglm::vec2(0.5f, 0.5f)});

	// Bottom cap vertices
	for (int32_t i = 0; i <= segments; ++i)
	{
		float theta = i * angleStep;
		alglm::vec3 position(radius * cos(theta), -halfHeight, radius * sin(theta));
		alglm::vec2 texCoord(0.5f + 0.5f * cos(theta), 0.5f + 0.5f * sin(theta));
		vertices.push_back({position, alglm::vec3(0.0f, -1.0f, 0.0f), texCoord});
	}

	std::vector<uint32_t> indices;

	// Top cap indices
	uint32_t topCenterIndex = 0;
	for (int32_t i = 1; i <= segments; ++i)
	{
		indices.push_back(topCenterIndex);
		indices.push_back(i + 1);
		indices.push_back(i);
	}

	// Bottom cap indices
	uint32_t bottomCenterIndex = segments + 2;
	for (int32_t i = 1; i <= segments; ++i)
	{
		indices.push_back(bottomCenterIndex);
		indices.push_back(bottomCenterIndex + i);
		indices.push_back(bottomCenterIndex + i + 1);
	}

	// Side indices
	for (int32_t i = 1; i <= segments; ++i)
	{
		uint32_t top1 = topCenterIndex + i;
		uint32_t top2 = topCenterIndex + i + 1;
		uint32_t bottom1 = bottomCenterIndex + i;
		uint32_t bottom2 = bottomCenterIndex + i + 1;

		// First triangle
		indices.push_back(top1);
		indices.push_back(bottom2);
		indices.push_back(bottom1);

		// Second triangle
		indices.push_back(bottom2);
		indices.push_back(top1);
		indices.push_back(top2);
	}

	return createMesh(vertices, indices);
}

std::shared_ptr<Mesh> Mesh::createColliderBox()
{
	std::vector<Vertex> vertices = {
		Vertex{alglm::vec3(-0.5f, -0.5f, -0.5f), alglm::vec3(0.0f, 0.0f, -1.0f), alglm::vec2(0.0f, 0.0f)},
		Vertex{alglm::vec3(0.5f, -0.5f, -0.5f), alglm::vec3(0.0f, 0.0f, -1.0f), alglm::vec2(1.0f, 0.0f)},
		Vertex{alglm::vec3(0.5f, 0.5f, -0.5f), alglm::vec3(0.0f, 0.0f, -1.0f), alglm::vec2(1.0f, 1.0f)},
		Vertex{alglm::vec3(-0.5f, 0.5f, -0.5f), alglm::vec3(0.0f, 0.0f, -1.0f), alglm::vec2(0.0f, 1.0f)},

		Vertex{alglm::vec3(-0.5f, -0.5f, 0.5f), alglm::vec3(0.0f, 0.0f, 1.0f), alglm::vec2(0.0f, 0.0f)},
		Vertex{alglm::vec3(0.5f, -0.5f, 0.5f), alglm::vec3(0.0f, 0.0f, 1.0f), alglm::vec2(1.0f, 0.0f)},
		Vertex{alglm::vec3(0.5f, 0.5f, 0.5f), alglm::vec3(0.0f, 0.0f, 1.0f), alglm::vec2(1.0f, 1.0f)},
		Vertex{alglm::vec3(-0.5f, 0.5f, 0.5f), alglm::vec3(0.0f, 0.0f, 1.0f), alglm::vec2(0.0f, 1.0f)},

		Vertex{alglm::vec3(-0.5f, 0.5f, 0.5f), alglm::vec3(-1.0f, 0.0f, 0.0f), alglm::vec2(1.0f, 0.0f)},
		Vertex{alglm::vec3(-0.5f, 0.5f, -0.5f), alglm::vec3(-1.0f, 0.0f, 0.0f), alglm::vec2(1.0f, 1.0f)},
		Vertex{alglm::vec3(-0.5f, -0.5f, -0.5f), alglm::vec3(-1.0f, 0.0f, 0.0f), alglm::vec2(0.0f, 1.0f)},
		Vertex{alglm::vec3(-0.5f, -0.5f, 0.5f), alglm::vec3(-1.0f, 0.0f, 0.0f), alglm::vec2(0.0f, 0.0f)},

		Vertex{alglm::vec3(0.5f, 0.5f, 0.5f), alglm::vec3(1.0f, 0.0f, 0.0f), alglm::vec2(1.0f, 0.0f)},
		Vertex{alglm::vec3(0.5f, 0.5f, -0.5f), alglm::vec3(1.0f, 0.0f, 0.0f), alglm::vec2(1.0f, 1.0f)},
		Vertex{alglm::vec3(0.5f, -0.5f, -0.5f), alglm::vec3(1.0f, 0.0f, 0.0f), alglm::vec2(0.0f, 1.0f)},
		Vertex{alglm::vec3(0.5f, -0.5f, 0.5f), alglm::vec3(1.0f, 0.0f, 0.0f), alglm::vec2(0.0f, 0.0f)},

		Vertex{alglm::vec3(-0.5f, -0.5f, -0.5f), alglm::vec3(0.0f, -1.0f, 0.0f), alglm::vec2(0.0f, 1.0f)},
		Vertex{alglm::vec3(0.5f, -0.5f, -0.5f), alglm::vec3(0.0f, -1.0f, 0.0f), alglm::vec2(1.0f, 1.0f)},
		Vertex{alglm::vec3(0.5f, -0.5f, 0.5f), alglm::vec3(0.0f, -1.0f, 0.0f), alglm::vec2(1.0f, 0.0f)},
		Vertex{alglm::vec3(-0.5f, -0.5f, 0.5f), alglm::vec3(0.0f, -1.0f, 0.0f), alglm::vec2(0.0f, 0.0f)},

		Vertex{alglm::vec3(-0.5f, 0.5f, -0.5f), alglm::vec3(0.0f, 1.0f, 0.0f), alglm::vec2(0.0f, 1.0f)},
		Vertex{alglm::vec3(0.5f, 0.5f, -0.5f), alglm::vec3(0.0f, 1.0f, 0.0f), alglm::vec2(1.0f, 1.0f)},
		Vertex{alglm::vec3(0.5f, 0.5f, 0.5f), alglm::vec3(0.0f, 1.0f, 0.0f), alglm::vec2(1.0f, 0.0f)},
		Vertex{alglm::vec3(-0.5f, 0.5f, 0.5f), alglm::vec3(0.0f, 1.0f, 0.0f), alglm::vec2(0.0f, 0.0f)},
	};

	std::vector<uint32_t> indices = {0,	 1,	 1,	 2,	 2,	 3,	 3,	 0,	 4,	 5,	 5,	 6,	 6,	 7,	 7,	 4,
									 8,	 9,	 9,	 10, 10, 11, 11, 8,	 12, 13, 13, 14, 14, 15, 15, 12,
									 16, 17, 17, 18, 18, 19, 19, 16, 20, 21, 21, 22, 22, 23, 23, 20};

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
	try
	{
		calculateTangents(vertices, indices);
		calculateAABB(vertices);
	}
	catch (const std::exception &e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
	}

	m_vertexBuffer = VertexBuffer::createVertexBuffer(vertices);
	m_indexBuffer = IndexBuffer::createIndexBuffer(indices);
}

void Mesh::calculateTangents(std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices)
{
	for (size_t i = 0; i < indices.size(); i += 3)
	{
		if (indices[i] >= vertices.size() || indices[i + 1] >= vertices.size() || indices[i + 2] >= vertices.size())
		{
			continue;
		}
		Vertex &v0 = vertices[indices[i]];
		Vertex &v1 = vertices[indices[i + 1]];
		Vertex &v2 = vertices[indices[i + 2]];

		alglm::vec3 edge1 = v1.pos - v0.pos;
		alglm::vec3 edge2 = v2.pos - v0.pos;

		alglm::vec2 deltaUV1 = v1.texCoord - v0.texCoord;
		alglm::vec2 deltaUV2 = v2.texCoord - v0.texCoord;

		float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
		if (std::isnan(f))
		{
			f = 0.0f;
		}

		alglm::vec3 tangent;
		tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
		tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
		tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

		tangent = alglm::normalize(tangent);

		v0.tangent += tangent;
		v1.tangent += tangent;
		v2.tangent += tangent;
	}

	// 정점 Tangent 정규화
	for (auto &vertex : vertices)
	{
		vertex.tangent = alglm::normalize(vertex.tangent);
	}
}

void Mesh::calculateAABB(std::vector<Vertex> &vertices)
{
	m_minPos = alglm::vec3(FLT_MAX);
	m_maxPos = alglm::vec3(-FLT_MAX);

	for (Vertex &vertex : vertices)
	{
		m_minPos = alglm::min(m_minPos, vertex.pos);
		m_maxPos = alglm::max(m_maxPos, vertex.pos);
	}
}

alglm::vec3 Mesh::getMaxPos()
{
	return m_maxPos;
}

alglm::vec3 Mesh::getMinPos()
{
	return m_minPos;
}

} // namespace ale