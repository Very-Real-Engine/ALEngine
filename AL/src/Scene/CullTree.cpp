#include "Scene/CullTree.h"
#include "Core/Log.h"
#include "Scene/Entity.h"

namespace ale
{

CullTree::CullTree()
{
	m_root = NULL_NODE;
	m_nodeCapacity = 16;
	m_nodes.resize(m_nodeCapacity);
	m_nodeCount = 0;

	for (int32_t i = 0; i < m_nodeCapacity - 1; ++i)
	{
		m_nodes[i].next = i + 1;
		m_nodes[i].height = -1;
	}
	m_nodes[m_nodeCapacity - 1].next = NULL_NODE;
	m_nodes[m_nodeCapacity - 1].height = -1;
	m_freeNode = 0;
}

void CullTree::updateTree()
{
	auto view = m_scene->getAllEntitiesWith<MeshRendererComponent, TransformComponent>();
	for (auto entity : view)
	{
		TransformComponent &transformComponent = view.get<TransformComponent>(entity);
		if (transformComponent.m_isMoved == true)
		{
			MeshRendererComponent &meshRendererComponent = view.get<MeshRendererComponent>(entity);
			CullSphere newSphere(transformComponent.m_WorldTransform *
									 alglm::vec4(meshRendererComponent.cullSphere.center, 1.0f),
								 transformComponent.getMaxScale() * meshRendererComponent.cullSphere.radius);
			moveNode(meshRendererComponent.nodeId, newSphere);
			transformComponent.m_isMoved = false;
		}
	}
}

int32_t CullTree::allocateNode()
{
	if (m_freeNode == NULL_NODE)
	{
		m_nodeCapacity *= 2;
		m_nodes.resize(m_nodeCapacity);

		for (int32_t i = m_nodeCount; i < m_nodeCapacity - 1; ++i)
		{
			m_nodes[i].next = i + 1;
			m_nodes[i].height = -1;
		}
		m_nodes[m_nodeCapacity - 1].next = NULL_NODE;
		m_nodes[m_nodeCapacity - 1].height = -1;
		m_freeNode = m_nodeCount;
	}

	int32_t nodeId = m_freeNode;
	m_freeNode = m_nodes[nodeId].next;
	m_nodes[nodeId].parent = NULL_NODE;
	m_nodes[nodeId].child1 = NULL_NODE;
	m_nodes[nodeId].child2 = NULL_NODE;
	m_nodes[nodeId].height = 0;
	m_nodes[nodeId].entityHandle = 0;
	++m_nodeCount;
	return nodeId;
}

void CullTree::freeNode(int32_t nodeId)
{
	m_nodes[nodeId].next = m_freeNode;
	m_nodes[nodeId].height = -1;
	m_freeNode = nodeId;
	--m_nodeCount;
}

int32_t CullTree::createNode(const CullSphere &sphere, uint32_t entityHandle)
{
	// AL_CORE_INFO("createNode!!");

	int32_t nodeId = allocateNode();
	// AL_CORE_INFO("nodeId = {}", nodeId);

	m_nodes[nodeId].sphere.center = sphere.center;
	m_nodes[nodeId].sphere.radius = sphere.radius;
	m_nodes[nodeId].entityHandle = entityHandle;
	m_nodes[nodeId].height = 0;

	// insert leaf
	insertLeaf(nodeId);

	return nodeId;
}

void CullTree::destroyNode(int32_t nodeId)
{
	detachNode(nodeId);
	freeNode(nodeId);
}

bool CullTree::moveNode(int32_t nodeId, const CullSphere &newSphere)
{
	detachNode(nodeId);

	m_nodes[nodeId].sphere = newSphere;

	insertLeaf(nodeId);
	return true;
}

void CullTree::setScene(Scene *scene)
{
	m_scene = scene;
}

void CullTree::setRenderEnable(int32_t nodeId)
{
	CullTreeNode &node = m_nodes[nodeId];

	if (node.isLeaf() == true)
	{
		MeshRendererComponent &component =
			m_scene->getComponent<MeshRendererComponent>(static_cast<entt::entity>(node.entityHandle));
		component.cullState = (component.cullState | ECullState::RENDER);
	}
	else
	{
		setRenderEnable(node.child1);
		setRenderEnable(node.child2);
	}
}

void CullTree::setRenderDisable(int32_t nodeId)
{
	CullTreeNode &node = m_nodes[nodeId];

	if (node.isLeaf() == true)
	{
		MeshRendererComponent &component =
			m_scene->getComponent<MeshRendererComponent>(static_cast<entt::entity>(node.entityHandle));
		component.cullState = (component.cullState & ECullState::NONE);
	}
	else
	{
		setRenderDisable(node.child1);
		setRenderDisable(node.child2);
	}
}

int32_t CullTree::getRootNodeId()
{
	return m_root;
}

void CullTree::frustumCulling(const Frustum &frustum, int32_t nodeId)
{
	CullTreeNode &node = m_nodes[nodeId];

	EFrustum result = frustum.cullingSphere(node.sphere);

	if (result == EFrustum::INSIDE)
	{
		// AL_CORE_INFO("node {} INSIDE!!", nodeId);
		setRenderEnable(nodeId);
	}
	else if (result == EFrustum::INTERSECT)
	{
		// AL_CORE_INFO("node {} INTERSECT!!", nodeId);
		if (node.isLeaf() == true)
		{
			setRenderEnable(nodeId);
		}
		else
		{
			frustumCulling(frustum, node.child1);
			frustumCulling(frustum, node.child2);
		}
	}
	else
	{
		// AL_CORE_INFO("node {} OUTSIDE!!",nodeId);
	}
}

void CullTree::insertLeaf(int32_t leaf)
{
	if (m_root == NULL_NODE)
	{
		m_root = leaf;
		m_nodes[m_root].parent = NULL_NODE;
		return;
	}

	CullSphere &leafSphere = m_nodes[leaf].sphere;
	int32_t index = m_root;

	while (m_nodes[index].isLeaf() == false)
	{
		int32_t child1 = m_nodes[index].child1;
		int32_t child2 = m_nodes[index].child2;

		float volume = m_nodes[index].sphere.getVolume();
		CullSphere combinedSphere;
		combinedSphere.combine(m_nodes[index].sphere, leafSphere);
		float combinedVolume = combinedSphere.getVolume();

		float parentCost = 2.0f * combinedVolume;
		float inheritedCost = 2.0f * (combinedVolume - volume);

		float childCost1;
		if (m_nodes[child1].isLeaf())
		{
			childCost1 = getInsertionCostForLeaf(leafSphere, child1, inheritedCost);
		}
		else
		{
			childCost1 = getInsertionCost(leafSphere, child1, inheritedCost);
		}

		float childCost2;
		if (m_nodes[child2].isLeaf())
		{
			childCost2 = getInsertionCostForLeaf(leafSphere, child2, inheritedCost);
		}
		else
		{
			childCost2 = getInsertionCost(leafSphere, child2, inheritedCost);
		}

		if (parentCost < childCost2 && parentCost < childCost2)
		{
			break;
		}

		if (childCost2 < childCost2)
		{
			index = child1;
		}
		else
		{
			index = child2;
		}
	}

	// 짝꿍
	int32_t sibling = index;

	int32_t oldParent = m_nodes[sibling].parent;
	int32_t newParent = allocateNode();

	m_nodes[newParent].parent = oldParent;
	m_nodes[newParent].entityHandle = 0;
	m_nodes[newParent].sphere.combine(leafSphere, m_nodes[sibling].sphere);
	m_nodes[newParent].height = m_nodes[sibling].height + 1;

	if (oldParent != NULL_NODE)
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
	while (index != NULL_NODE)
	{
		index = balance(index);

		int32_t child1 = m_nodes[index].child1;
		int32_t child2 = m_nodes[index].child2;

		assert(child1 != NULL_NODE);
		assert(child2 != NULL_NODE);

		m_nodes[index].height = std::max(m_nodes[child1].height, m_nodes[child2].height) + 1;
		m_nodes[index].sphere.combine(m_nodes[child1].sphere, m_nodes[child2].sphere);

		index = m_nodes[index].parent;
	}
	// std::cout << "print tree\n";
	// printDynamicTree(root);
}

void CullTree::detachNode(int32_t nodeId)
{
	if (nodeId == m_root)
	{
		m_root = NULL_NODE;
		return;
	}

	int32_t parent = m_nodes[nodeId].parent;
	int32_t grandParent = m_nodes[parent].parent;
	int32_t sibling;

	if (m_nodes[parent].child1 == nodeId)
	{
		sibling = m_nodes[parent].child2;
	}
	else
	{
		sibling = m_nodes[parent].child1;
	}

	if (grandParent != NULL_NODE)
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
		while (index != NULL_NODE)
		{
			index = balance(index);

			int32_t child1 = m_nodes[index].child1;
			int32_t child2 = m_nodes[index].child2;

			m_nodes[index].height = std::max(m_nodes[child1].height, m_nodes[child2].height) + 1;
			m_nodes[index].sphere.combine(m_nodes[child1].sphere, m_nodes[child2].sphere);

			index = m_nodes[index].parent;
		}
	}
	else
	{
		m_root = sibling;
		m_nodes[sibling].parent = NULL_NODE;
		freeNode(parent);
	}
	// std::cout << "print tree\n";
	// printDynamicTree(root);
}

float CullTree::getInsertionCostForLeaf(const CullSphere &leafSphere, int32_t child, float inheritedCost)
{
	CullSphere sphere;
	sphere.combine(leafSphere, m_nodes[child].sphere);
	return sphere.getVolume() + inheritedCost;
}

float CullTree::getInsertionCost(const CullSphere &leafSphere, int32_t child, float inheritedCost)
{
	CullSphere sphere;
	sphere.combine(leafSphere, m_nodes[child].sphere);
	float oldVolume = m_nodes[child].sphere.getVolume();
	float newVolume = sphere.getVolume();
	return (newVolume - oldVolume) + inheritedCost;
}

void CullTree::printCullTree(int32_t nodeId)
{

	if (nodeId == NULL_NODE)
	{
		return;
	}

	AL_CORE_INFO("nodeId: {} start", nodeId);
	// AL_CORE_INFO("Sphere center: {}, {}, {}", m_nodes[nodeId].sphere.center.x, m_nodes[nodeId].sphere.center.y,
	// 			 m_nodes[nodeId].sphere.center.z);
	// AL_CORE_INFO("Sphere radius: {}", m_nodes[nodeId].sphere.radius);
	AL_CORE_INFO("entityHandle: {}", m_nodes[nodeId].entityHandle);
	AL_CORE_INFO("child1: {}", m_nodes[nodeId].child1);
	AL_CORE_INFO("child2: {}", m_nodes[nodeId].child2);
	// AL_CORE_INFO("height: {}", m_nodes[nodeId].height);
	printCullTree(m_nodes[nodeId].child1);
	printCullTree(m_nodes[nodeId].child2);
}

int32_t CullTree::getSize()
{
	return m_nodeCount;
}

int32_t CullTree::balance(int32_t iA)
{
	CullTreeNode *A = &m_nodes[iA];

	if (A->isLeaf() || A->height < 2)
	{
		return iA;
	}

	int32_t iB = A->child1;
	int32_t iC = A->child2;

	CullTreeNode *B = &m_nodes[iB];
	CullTreeNode *C = &m_nodes[iC];

	int32_t balance = C->height - B->height;

	if (balance > 1)
	{
		int32_t iF = m_nodes[iC].child1;
		int32_t iG = m_nodes[iC].child2;

		CullTreeNode *F = &m_nodes[iF];
		CullTreeNode *G = &m_nodes[iG];

		C->child1 = iA;
		C->parent = A->parent;
		A->parent = iC;

		if (C->parent != NULL_NODE)
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
			A->sphere.combine(B->sphere, G->sphere);
			C->sphere.combine(A->sphere, F->sphere);

			A->height = std::max(B->height, G->height) + 1;
			C->height = std::max(A->height, F->height) + 1;
		}
		else
		{
			C->child2 = iG;
			A->child2 = iF;
			F->parent = iA;
			A->sphere.combine(B->sphere, F->sphere);
			C->sphere.combine(A->sphere, G->sphere);

			A->height = std::max(B->height, F->height) + 1;
			C->height = std::max(A->height, G->height) + 1;
		}
		return iC;
	}

	if (balance < -1)
	{
		int32_t iD = m_nodes[iB].child1;
		int32_t iE = m_nodes[iB].child2;

		CullTreeNode *D = &m_nodes[iD];
		CullTreeNode *E = &m_nodes[iE];

		B->child1 = iA;
		B->parent = A->parent;
		A->parent = iB;

		if (B->parent != NULL_NODE)
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
			A->sphere.combine(C->sphere, E->sphere);
			B->sphere.combine(A->sphere, D->sphere);

			A->height = std::max(C->height, E->height) + 1;
			B->height = std::max(A->height, D->height) + 1;
		}
		else
		{
			B->child2 = iE;
			A->child1 = iD;
			D->parent = iA;
			A->sphere.combine(C->sphere, D->sphere);
			B->sphere.combine(A->sphere, E->sphere);

			A->height = std::max(C->height, D->height) + 1;
			B->height = std::max(A->height, E->height) + 1;
		}
		return iB;
	}
	return iA;
}

void CullTree::changeEntityHandle(int32_t nodeId, uint32_t entityHandle)
{
	m_nodes[nodeId].entityHandle = entityHandle;
}

ECullState operator&(ECullState state1, ECullState state2)
{
	int32_t s1 = static_cast<int32_t>(state1);
	int32_t s2 = static_cast<int32_t>(state2);
	return static_cast<ECullState>(s1 & s2);
}

ECullState operator|(ECullState state1, ECullState state2)
{
	int32_t s1 = static_cast<int32_t>(state1);
	int32_t s2 = static_cast<int32_t>(state2);
	return static_cast<ECullState>(s1 | s2);
}

} // namespace ale