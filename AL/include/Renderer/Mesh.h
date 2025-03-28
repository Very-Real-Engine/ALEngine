#pragma once

/**
 * @file Mesh.h
 * @brief Mesh 클래스
 */

#include "Core/Base.h"
#include "Renderer/Buffer.h"
#include "Renderer/Common.h"

namespace ale
{
/**
 * @brief Mesh 클래스
 */
class Mesh
{
  public:
	/**
	 * @brief Mesh 생성
	 * @param vertices 정점 목록
	 * @param indices 인덱스 목록
	 * @param globalTransform 노드별 글로벌 위치, 없다면 항등행렬 설정
	 * @return Mesh
	 */
	static std::shared_ptr<Mesh> createMesh(std::vector<Vertex> &vertices, std::vector<uint32_t> &indices,
											const alglm::mat4 &globalTransform = alglm::mat4(1.0f));
	/**
	 * @brief 박스 생치치
	 * @return 박스
	 */
	static std::shared_ptr<Mesh> createBox();
	/**
	 * @brief 구 생성
	 * @return 구
	 */
	static std::shared_ptr<Mesh> createSphere();
	/**
	 * @brief 평면 생성
	 * @return 평면
	 */
	static std::shared_ptr<Mesh> createPlane();
	/**
	 * @brief 지면 생성
	 * @return 지면
	 */
	static std::shared_ptr<Mesh> createGround();
	/**
	 * @brief 캡슐 생성
	 * @return 캡슐
	 */
	static std::shared_ptr<Mesh> createCapsule();
	/**
	 * @brief 원기둥 생성
	 * @return 원기둥
	 */
	static std::shared_ptr<Mesh> createCylinder();
	/**
	 * @brief 콜라이더 박스 생성
	 * @return 콜라이더 박스
	 */
	static std::shared_ptr<Mesh> createColliderBox();

	~Mesh() = default;

	/**
	 * @brief Mesh 정리
	 */
	void cleanup();

	/**
	 * @brief Mesh 그리기
	 * @param commandBuffer 명령 버퍼
	 */
	void draw(VkCommandBuffer commandBuffer);

	void drawShadowSSBO(VkCommandBuffer commandBuffer, uint32_t instanceCount, uint32_t firstInstance);

	/**
	 * @brief AABB 계산
	 * @param vertices 정점 목록
	 */
	void calculateAABB(std::vector<Vertex> &vertices);
	/**
	 * @brief 최대 위치 반환
	 * @return 최대 위치
	 */
	alglm::vec3 getMaxPos();
	/**
	 * @brief 최소 위치 반환
	 * @return 최소 위치
	 */
	alglm::vec3 getMinPos();
	/**
	 * @brief 노드 글로벌 트랜스폼 반환
	 * @return alglm::mat4
	 */
	alglm::mat4 getNodeTransform();

	/**
	 * @brief Mesh ID 반환
	 * @return Mesh ID
	 */
	uint32_t getId()
	{
		return m_id;
	}

  private:
	Mesh() = default;

	alglm::vec3 m_minPos;
	alglm::vec3 m_maxPos;
	alglm::mat4 m_GlobalTransform;
	std::unique_ptr<VertexBuffer> m_vertexBuffer;
	std::unique_ptr<IndexBuffer> m_indexBuffer;
	uint32_t m_id;

	/**
	 * @brief Mesh 초기화
	 * @param vertices 정점 목록
	 * @param indices 인덱스 목록
	 */
	void initMesh(std::vector<Vertex> &vertices, std::vector<uint32_t> &indices);
	/**
	 * @brief 탄젠트 계산
	 * @param vertices 정점 목록
	 * @param indices 인덱스 목록
	 */
	void calculateTangents(std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices);
};
} // namespace ale
