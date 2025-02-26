#pragma once

#include "Physics/Collision.h"

#include <stack>

#define nullNode (-1)

namespace ale
{
/**
 * @file
 * @brief 동적 충돌 감지를 위한 DynamicTree 클래스 및 관련 구조체 정의.
 */

/**
 * @struct TreeNode
 * @brief 동적 트리의 노드를 정의하는 구조체.
 * @details AABB(축 정렬 바운딩 박스)와 트리의 계층 구조를 관리합니다.
 */
struct TreeNode
{
	/**
	 * @brief 노드가 리프(Leaf)인지 확인합니다.
	 * @return 리프 노드이면 true, 그렇지 않으면 false.
	 */
	bool isLeaf() const
	{
		return child1 == nullNode;
	}
	AABB aabb; // Enlarged AABB
	void *userData;
	union {
		int32_t parent;
		int32_t next;
	};
	int32_t child1;
	int32_t child2;
	int32_t height;
};

/**
 * @class DynamicTree
 * @brief 동적 트리 기반 충돌 감지를 수행하는 클래스.
 * @details DynamicTree는 AABB 트리를 관리하여 빠른 충돌 탐색을 지원합니다.
 */
class DynamicTree
{
  public:
	/**
	 * @brief DynamicTree 기본 생성자.
	 */
	DynamicTree();

	/**
	 * @brief DynamicTree 소멸자.
	 */
	~DynamicTree();

	/**
	 * @brief AABB와 사용자 데이터를 기반으로 새로운 Proxy(노드)를 생성합니다.
	 * @param aabb 삽입할 AABB.
	 * @param userData 사용자 데이터 포인터.
	 * @return 생성된 Proxy ID.
	 */
	int32_t createProxy(const AABB &aabb, void *userData);

	/**
	 * @brief Proxy ID에 해당하는 노드를 제거합니다.
	 * @param proxyId 제거할 Proxy ID.
	 */
	void destroyProxy(int32_t proxyId);

	/**
	 * @brief Proxy를 새로운 위치로 이동하고 트리를 재구성합니다.
	 * @param proxyId 이동할 Proxy ID.
	 * @param aabb 새로운 AABB.
	 * @param displacement 이동 벡터.
	 * @return 이동 성공 여부 (true: 성공, false: 실패).
	 */
	bool moveProxy(int32_t proxyId, const AABB &aabb, const alglm::vec3 &displacement);

	/**
	 * @brief Proxy ID에 해당하는 사용자 데이터를 반환합니다.
	 * @param proxyId 조회할 Proxy ID.
	 * @return 사용자 데이터 포인터.
	 */
	void *getUserData(int32_t proxyId) const;

	/**
	 * @brief Proxy ID에 해당하는 AABB를 반환합니다.
	 * @param proxyId 조회할 Proxy ID.
	 * @return 해당 Proxy의 AABB.
	 */
	const AABB &getFatAABB(int32_t proxyId) const;

	/**
	 * @brief AABB와 겹치는 노드를 탐색합니다.
	 * @tparam T 콜백 함수 타입.
	 * @param callback 검색된 노드를 처리할 콜백 함수.
	 * @param aabb 검색할 AABB 영역.
	 */
	template <typename T> void query(T *callback, const AABB &aabb) const;

  private:
	/**
	 * @brief 새로운 노드를 할당합니다.
	 * @return 할당된 노드 ID.
	 */
	int32_t allocateNode();

	/**
	 * @brief 특정 노드를 해제합니다.
	 * @param nodeId 해제할 노드 ID.
	 */
	void freeNode(int32_t nodeId);

	/**
	 * @brief 노드를 트리에 삽입하고 균형을 조정합니다.
	 * @param leaf 삽입할 리프 노드 ID.
	 */
	void insertLeaf(int32_t leaf);

	/**
	 * @brief 노드를 트리에서 제거합니다.
	 * @param leaf 제거할 리프 노드 ID.
	 */
	void removeLeaf(int32_t leaf);

	/**
	 * @brief 트리 균형을 조정합니다.
	 * @param index 균형을 조정할 노드 인덱스.
	 * @return 균형이 맞춰진 노드 인덱스.
	 */
	int32_t balance(int32_t index);

	/**
	 * @brief 특정 리프 노드의 삽입 비용을 계산합니다.
	 * @param leafAABB 삽입할 AABB.
	 * @param child 대상 노드.
	 * @param inheritedCost 상속된 비용.
	 * @return 삽입 비용.
	 */
	float getInsertionCostForLeaf(const AABB &leafAABB, int32_t child, float inheritedCost);

	/**
	 * @brief 특정 노드의 삽입 비용을 계산합니다.
	 * @param leafAABB 삽입할 AABB.
	 * @param child 대상 노드.
	 * @param inheritedCost 상속된 비용.
	 * @return 삽입 비용.
	 */
	float getInsertionCost(const AABB &leafAABB, int32_t child, float inheritedCost);

	/**
	 * @brief 동적 트리의 구조를 출력합니다.
	 * @param node 출력할 노드의 인덱스.
	 */
	void printDynamicTree(int32_t node);

	// // tree의 height 계산
	// int32_t ComputeHeight() const;
	// // sub-tree의 height 계산
	// int32_t ComputeHeight(int32_t nodeId) const;

	int32_t m_root;
	int32_t m_freeNode;
	int32_t m_nodeCount;
	int32_t m_nodeCapacity;
	std::vector<TreeNode> m_nodes;
};

template <typename T> inline void DynamicTree::query(T *callback, const AABB &aabb) const
{
	std::stack<int32_t> stack;
	stack.push(m_root);

	while (!stack.empty())
	{
		int32_t nodeId = stack.top();
		stack.pop();
		if (nodeId == nullNode)
		{
			continue;
		}

		const TreeNode node = m_nodes[nodeId];
		if (testOverlap(node.aabb, aabb))
		{
			if (node.isLeaf())
			{
				bool proceed = callback->queryCallback(nodeId);
				if (proceed == false)
				{
					return;
				}
			}
			else
			{
				stack.push(node.child1);
				stack.push(node.child2);
			}
		}
	}
}
} // namespace ale