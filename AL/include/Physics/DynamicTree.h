#pragma once

#include "Physics/Collision.h"

#include <stack>

#define nullNode (-1)

namespace ale
{
struct TreeNode
{
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

class DynamicTree
{
  public:
	// Dynamic Tree 생성
	DynamicTree();
	~DynamicTree();

	// 주어진 aabb와 userData로 node에 값 초기화, node 삽입
	int32_t createProxy(const AABB &aabb, void *userData);

	// proxyId에 해당하는 node Destroy
	void destroyProxy(int32_t proxyId);

	// proxyId에 해당하는 node 삭제 후, 적당한 위치로 다시 Insert
	bool moveProxy(int32_t proxyId, const AABB &aabb, const alglm::vec3 &displacement);

	//
	void *getUserData(int32_t proxyId) const;

	//
	const AABB &getFatAABB(int32_t proxyId) const;

	template <typename T> void query(T *callback, const AABB &aabb) const;

  private:
	int32_t allocateNode();
	void freeNode(int32_t nodeId);

	// 가장 최적의 위치를 찾아 node 삽입, balance
	void insertLeaf(int32_t leaf);

	// node 삭제
	void removeLeaf(int32_t leaf);

	// 트리가 쏠리지 않게 balance 맞춰줌
	int32_t balance(int32_t index);

	float getInsertionCostForLeaf(const AABB &leafAABB, int32_t child, float inheritedCost);

	float getInsertionCost(const AABB &leafAABB, int32_t child, float inheritedCost);

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
}