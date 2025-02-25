#include "Physics/DynamicTree.h"

namespace ale
{
DynamicTree::DynamicTree()
{
	m_root = nullNode;
	m_nodeCapacity = 16;
	m_nodes.resize(m_nodeCapacity);
	m_nodeCount = 0;

	for (int32_t i = 0; i < m_nodeCapacity - 1; ++i)
	{
		m_nodes[i].next = i + 1;
		m_nodes[i].height = -1;
	}
	m_nodes[m_nodeCapacity - 1].next = nullNode;
	m_nodes[m_nodeCapacity - 1].height = -1;
	m_freeNode = 0;
}

DynamicTree::~DynamicTree()
{
	m_nodes.clear();
}

int DynamicTree::allocateNode()
{
	if (m_freeNode == nullNode)
	{
		m_nodeCapacity *= 2;
		m_nodes.resize(m_nodeCapacity);

		for (int32_t i = m_nodeCount; i < m_nodeCapacity - 1; ++i)
		{
			m_nodes[i].next = i + 1;
			m_nodes[i].height = -1;
		}
		m_nodes[m_nodeCapacity - 1].next = nullNode;
		m_nodes[m_nodeCapacity - 1].height = -1;
		m_freeNode = m_nodeCount;
	}

	int32_t nodeId = m_freeNode;
	m_freeNode = m_nodes[nodeId].next;
	m_nodes[nodeId].parent = nullNode;
	m_nodes[nodeId].child1 = nullNode;
	m_nodes[nodeId].child2 = nullNode;
	m_nodes[nodeId].height = 0;
	m_nodes[nodeId].userData = nullptr;
	++m_nodeCount;
	return nodeId;
}

void DynamicTree::freeNode(int32_t nodeId)
{
	m_nodes[nodeId].next = m_freeNode;
	m_nodes[nodeId].height = -1;
	m_freeNode = nodeId;
	--m_nodeCount;
}

int32_t DynamicTree::createProxy(const AABB &aabb, void *userData)
{
	int32_t proxyId = allocateNode();

	alglm::vec3 r(0.1, 0.1, 0.1);
	m_nodes[proxyId].aabb.lowerBound = aabb.lowerBound - r;
	m_nodes[proxyId].aabb.upperBound = aabb.upperBound + r;
	m_nodes[proxyId].userData = userData;
	m_nodes[proxyId].height = 0;

	// insert leaf
	insertLeaf(proxyId);

	return proxyId;
}

void DynamicTree::destroyProxy(int32_t proxyId)
{
	// remove leaf
	freeNode(proxyId);
}

bool DynamicTree::moveProxy(int32_t proxyId, const AABB &aabb, const alglm::vec3 &displacement)
{
	if (m_nodes[proxyId].aabb.contains(aabb))
	{
		return false;
	}

	removeLeaf(proxyId);

	AABB b = aabb;
	alglm::vec3 r(0.1f);
	b.lowerBound = b.lowerBound - r;
	b.upperBound = b.upperBound + r;

	alglm::vec3 d = 2.0f * displacement;

	if (d.x < 0.0f)
	{
		b.lowerBound.x += d.x;
	}
	else
	{
		b.upperBound.x += d.x;
	}

	if (d.y < 0.0f)
	{
		b.lowerBound.y += d.y;
	}
	else
	{
		b.upperBound.y += d.y;
	}

	if (d.z < 0.0f)
	{
		b.lowerBound.z += d.z;
	}
	else
	{
		b.upperBound.z += d.z;
	}

	m_nodes[proxyId].aabb = b;

	insertLeaf(proxyId);
	return true;
}

void *DynamicTree::getUserData(int32_t proxyId) const
{
	// check proxyId range
	return m_nodes[proxyId].userData;
}

const AABB &DynamicTree::getFatAABB(int32_t proxyId) const
{
	return m_nodes[proxyId].aabb;
}

void DynamicTree::insertLeaf(int32_t leaf)
{
	if (m_root == nullNode)
	{
		m_root = leaf;
		m_nodes[m_root].parent = nullNode;
		return;
	}

	AABB leafAABB = m_nodes[leaf].aabb;
	int32_t index = m_root;

	while (m_nodes[index].isLeaf() == false)
	{
		int32_t child1 = m_nodes[index].child1;
		int32_t child2 = m_nodes[index].child2;

		float area = m_nodes[index].aabb.getSurface();
		AABB combinedAABB;
		combinedAABB.combine(m_nodes[index].aabb, leafAABB);
		float combinedArea = combinedAABB.getSurface();

		float cost = 2.0f * combinedArea;
		float inheritedCost = 2.0f * (combinedArea - area);

		float cost1;
		if (m_nodes[child1].isLeaf())
		{
			cost1 = getInsertionCostForLeaf(leafAABB, child1, inheritedCost);
		}
		else
		{
			cost1 = getInsertionCost(leafAABB, child1, inheritedCost);
		}

		float cost2;
		if (m_nodes[child2].isLeaf())
		{
			cost2 = getInsertionCostForLeaf(leafAABB, child2, inheritedCost);
		}
		else
		{
			cost2 = getInsertionCost(leafAABB, child2, inheritedCost);
		}

		if (cost < cost1 && cost < cost2)
			break;

		if (cost1 < cost2)
		{
			index = child1;
		}
		else
		{
			index = child2;
		}
	}

	int32_t sibling = index;

	int32_t oldParent = m_nodes[sibling].parent;
	int32_t newParent = allocateNode();

	m_nodes[newParent].parent = oldParent;
	m_nodes[newParent].userData = nullptr;
	m_nodes[newParent].aabb.combine(leafAABB, m_nodes[sibling].aabb);
	m_nodes[newParent].height = m_nodes[sibling].height + 1;

	if (oldParent != nullNode)
	{
		if (m_nodes[oldParent].child1 == sibling)
		{
			m_nodes[oldParent].child1 = newParent;
		}
		else
		{
			m_nodes[oldParent].child2 = newParent;
		}

		m_nodes[newParent].child1 = sibling;
		m_nodes[newParent].child2 = leaf;
		m_nodes[sibling].parent = newParent;
		m_nodes[leaf].parent = newParent;
	}
	else
	{
		m_nodes[newParent].child1 = sibling;
		m_nodes[newParent].child2 = leaf;
		m_nodes[sibling].parent = newParent;
		m_nodes[leaf].parent = newParent;
		m_root = newParent;
	}

	index = m_nodes[leaf].parent;
	while (index != nullNode)
	{
		index = balance(index);

		int32_t child1 = m_nodes[index].child1;
		int32_t child2 = m_nodes[index].child2;

		assert(child1 != nullNode);
		assert(child2 != nullNode);

		m_nodes[index].height = std::max(m_nodes[child1].height, m_nodes[child2].height) + 1;
		m_nodes[index].aabb.combine(m_nodes[child1].aabb, m_nodes[child2].aabb);

		index = m_nodes[index].parent;
	}
	// std::cout << "print tree\n";
	// printDynamicTree(root);
}

void DynamicTree::removeLeaf(int32_t leaf)
{
	if (leaf == m_root)
	{
		m_root = nullNode;
		return;
	}

	int32_t parent = m_nodes[leaf].parent;
	int32_t grandParent = m_nodes[parent].parent;
	int32_t sibling;

	if (m_nodes[parent].child1 == leaf)
	{
		sibling = m_nodes[parent].child2;
	}
	else
	{
		sibling = m_nodes[parent].child1;
	}

	if (grandParent != nullNode)
	{
		if (m_nodes[grandParent].child1 == parent)
		{
			m_nodes[grandParent].child1 = sibling;
		}
		else
		{
			m_nodes[grandParent].child2 = sibling;
		}
		m_nodes[sibling].parent = grandParent;
		freeNode(parent);

		int32_t index = grandParent;
		while (index != nullNode)
		{
			index = balance(index);

			int32_t child1 = m_nodes[index].child1;
			int32_t child2 = m_nodes[index].child2;

			m_nodes[index].height = std::max(m_nodes[child1].height, m_nodes[child2].height) + 1;
			m_nodes[index].aabb.combine(m_nodes[child1].aabb, m_nodes[child2].aabb);

			index = m_nodes[index].parent;
		}
	}
	else
	{
		m_root = sibling;
		m_nodes[sibling].parent = nullNode;
		freeNode(parent);
	}
}

float DynamicTree::getInsertionCostForLeaf(const AABB &leafAABB, int32_t child, float inheritedCost)
{
	AABB aabb;
	aabb.combine(leafAABB, m_nodes[child].aabb);
	return aabb.getSurface() + inheritedCost;
}

float DynamicTree::getInsertionCost(const AABB &leafAABB, int32_t child, float inheritedCost)
{
	AABB aabb;
	aabb.combine(leafAABB, m_nodes[child].aabb);
	float oldArea = m_nodes[child].aabb.getSurface();
	float newArea = aabb.getSurface();
	return (newArea - oldArea) + inheritedCost;
}

void DynamicTree::printDynamicTree(int32_t nodeId)
{
	if (nodeId == nullNode)
	{
		return;
	}
	std::cout << nodeId << "\n";
	printDynamicTree(m_nodes[nodeId].child1);
	printDynamicTree(m_nodes[nodeId].child2);
}

int32_t DynamicTree::balance(int32_t iA)
{
	TreeNode *A = &m_nodes[iA];

	if (A->isLeaf() || A->height < 2)
	{
		return iA;
	}

	int32_t iB = A->child1;
	int32_t iC = A->child2;

	TreeNode *B = &m_nodes[iB];
	TreeNode *C = &m_nodes[iC];

	int32_t balance = C->height - B->height;

	if (balance > 1)
	{
		int32_t iF = m_nodes[iC].child1;
		int32_t iG = m_nodes[iC].child2;

		TreeNode *F = &m_nodes[iF];
		TreeNode *G = &m_nodes[iG];

		C->child1 = iA;
		C->parent = A->parent;
		A->parent = iC;

		if (C->parent != nullNode)
		{
			if (m_nodes[C->parent].child1 == iA)
			{
				m_nodes[C->parent].child1 = iC;
			}
			else
			{
				m_nodes[C->parent].child2 = iC;
			}
		}
		else
		{
			m_root = iC;
		}

		if (F->height > G->height)
		{
			C->child2 = iF;
			A->child2 = iG;
			G->parent = iA;
			m_nodes[iG].parent = iA;
			A->aabb.combine(B->aabb, G->aabb);
			C->aabb.combine(A->aabb, F->aabb);

			A->height = std::max(B->height, G->height) + 1;
			C->height = std::max(A->height, F->height) + 1;
		}
		else
		{
			C->child2 = iG;
			A->child2 = iF;
			F->parent = iA;
			A->aabb.combine(B->aabb, F->aabb);
			C->aabb.combine(A->aabb, G->aabb);

			A->height = std::max(B->height, F->height) + 1;
			C->height = std::max(A->height, G->height) + 1;
		}
		return iC;
	}

	if (balance < -1)
	{
		int32_t iD = m_nodes[iB].child1;
		int32_t iE = m_nodes[iB].child2;

		TreeNode *D = &m_nodes[iD];
		TreeNode *E = &m_nodes[iE];

		B->child1 = iA;
		B->parent = A->parent;
		A->parent = iB;

		if (B->parent != nullNode)
		{
			if (m_nodes[B->parent].child1 == iA)
			{
				m_nodes[B->parent].child1 = iB;
			}
			else
			{
				m_nodes[B->parent].child2 = iB;
			}
		}
		else
		{
			m_root = iB;
		}

		if (D->height > E->height)
		{
			B->child2 = iD;
			A->child1 = iE;
			E->parent = iA;
			A->aabb.combine(C->aabb, E->aabb);
			B->aabb.combine(A->aabb, D->aabb);

			A->height = std::max(C->height, E->height) + 1;
			B->height = std::max(A->height, D->height) + 1;
		}
		else
		{
			B->child2 = iE;
			A->child1 = iD;
			D->parent = iA;
			A->aabb.combine(C->aabb, D->aabb);
			B->aabb.combine(A->aabb, E->aabb);

			A->height = std::max(C->height, D->height) + 1;
			B->height = std::max(A->height, E->height) + 1;
		}
		return iB;
	}
	return iA;
}
} // namespace ale