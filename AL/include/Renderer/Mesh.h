#ifndef MESH_H
#define MESH_H

#include "Core/Base.h"
#include "Renderer/Buffer.h"
#include "Renderer/Common.h"

namespace ale
{
class Mesh
{
  public:
	static std::shared_ptr<Mesh> createMesh(std::vector<Vertex> &vertices, std::vector<uint32_t> &indices);
	static std::shared_ptr<Mesh> createBox();
	static std::shared_ptr<Mesh> createSphere();
	static std::shared_ptr<Mesh> createPlane();
	static std::shared_ptr<Mesh> createGround();

	~Mesh() = default;

	void cleanup();

	void draw(VkCommandBuffer commandBuffer);

	void calculateAABB(std::vector<Vertex> &vertices);
	glm::vec3 getMaxPos();
	glm::vec3 getMinPos();

  private:
	Mesh() = default;

	glm::vec3 m_minPos;
	glm::vec3 m_maxPos;
	std::unique_ptr<VertexBuffer> m_vertexBuffer;
	std::unique_ptr<IndexBuffer> m_indexBuffer;

	void initMesh(std::vector<Vertex> &vertices, std::vector<uint32_t> &indices);
	void calculateTangents(std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices);
};
} // namespace ale

#endif