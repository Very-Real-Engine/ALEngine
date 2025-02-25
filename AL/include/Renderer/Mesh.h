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
	static std::shared_ptr<Mesh> createCapsule();
	static std::shared_ptr<Mesh> createCylinder();

	~Mesh() = default;

	void cleanup();

	void draw(VkCommandBuffer commandBuffer);

	void calculateAABB(std::vector<Vertex> &vertices);
	alglm::vec3 getMaxPos();
	alglm::vec3 getMinPos();

  private:
	Mesh() = default;

	alglm::vec3 m_minPos;
	alglm::vec3 m_maxPos;
	std::unique_ptr<VertexBuffer> m_vertexBuffer;
	std::unique_ptr<IndexBuffer> m_indexBuffer;

	void initMesh(std::vector<Vertex> &vertices, std::vector<uint32_t> &indices);
	void calculateTangents(std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices);
};
} // namespace ale

#endif