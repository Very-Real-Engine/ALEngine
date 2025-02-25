#pragma once

#include "Core/Log.h"
#include "Renderer/Common.h"

namespace ale
{

#define NULL_NODE (-1)

enum class ECullState
{
	CULL = 0,
	RENDER = (1 << 0),
	NONE = (1 << 1),
	RENDER_AND_NONE = (1 << 0) | (1 << 1),
};

ECullState operator&(ECullState state1, ECullState state2);
ECullState operator|(ECullState state1, ECullState state2);

struct CullSphere
{
	alglm::vec3 center;
	float radius;

	CullSphere() = default;

	CullSphere(alglm::vec3 &center, float radius) : center(center), radius(radius) {};

	CullSphere(alglm::vec4 &center, float radius) : center(center), radius(radius) {};

	float getVolume()
	{
		return radius * radius * radius;
	}

	void combine(const CullSphere &sphere1, const CullSphere &sphere2)
	{
		alglm::vec3 centerDiff = sphere2.center - sphere1.center;
		float distance = alglm::length(centerDiff);

		// sphere1이 sphere2를 포함
		if (sphere1.radius >= distance + sphere2.radius)
		{
			center = sphere1.center;
			radius = sphere1.radius;
			return;
		}

		// sphere2가 sphere1을 포함
		if (sphere2.radius >= distance + sphere1.radius)
		{
			center = sphere2.center;
			radius = sphere2.radius;
			return;
		}

		radius = (distance + sphere1.radius + sphere2.radius) * 0.5f;

		// 새로운 중심: 기존 중심을 거리 비율로 보간하여 위치 지정
		center = sphere1.center;
		center += centerDiff * ((radius - sphere1.radius) / distance);
	}
};

struct FrustumPlane
{
	float distance;
	alglm::vec3 normal;

	FrustumPlane() = default;

	// CCW
	FrustumPlane(const alglm::vec3 &p1, const alglm::vec3 &p2, const alglm::vec3 &p3)
	{
		normal = alglm::normalize(alglm::cross(p2 - p1, p3 - p1));
		distance = alglm::dot(normal, p1);
		// AL_CORE_INFO("p1: {}, {}, {}", p1.x, p1.y, p1.z);
		// AL_CORE_INFO("normal: {}, {}, {}", normal.x, normal.y, normal.z);
		// AL_CORE_INFO("distance: {}", distance);
	}
};

enum class EFrustum
{
	OUTSIDE,
	INSIDE,
	INTERSECT,
};

struct Frustum
{
	// 0: near, 1: far, 2: left, 3: right, 4: up, 5: down
	FrustumPlane plane[6];

	EFrustum cullingSphere(const CullSphere &sphere) const
	{
		bool intersect = false;

		for (int32_t i = 0; i < 6; ++i)
		{
			float dotResult = alglm::dot(plane[i].normal, sphere.center);
			float distance = plane[i].distance;
			// AL_CORE_INFO("sphere center: {}, {}, {}", sphere.center.x, sphere.center.y, sphere.center.z);
			// AL_CORE_INFO("sphere radius: {}", sphere.radius);
			// AL_CORE_INFO("plane[{}]", i);
			// AL_CORE_INFO("plane normal: {}, {}, {}", plane[i].normal.x, plane[i].normal.y, plane[i].normal.z);
			// AL_CORE_INFO("plane distance: {}", plane[i].distance);
			// AL_CORE_INFO("dotResult: {}", dotResult);
			// AL_CORE_INFO("distance: {}", distance);
			// AL_CORE_INFO("distance + radius: {}", sphere.radius + distance);
			if (dotResult - sphere.radius > distance)
			{
				// AL_CORE_INFO("OUT!!!!!!!!!!!");
				return EFrustum::OUTSIDE;
			}
			else if (dotResult + sphere.radius > distance)
			{
				intersect = true;
			}
		}

		if (intersect == true)
		{
			return EFrustum::INTERSECT;
		}
		else
		{
			return EFrustum::INSIDE;
		}
	}
};

struct CullTreeNode
{
	bool isLeaf() const
	{
		return child1 == NULL_NODE;
	}

	CullSphere sphere;
	uint32_t entityHandle;

	union {
		int32_t parent;
		int32_t next;
	};
	int32_t child1;
	int32_t child2;
	int32_t height;
};

class Scene;

class CullTree
{
  public:
	// Dynamic Tree 생성
	CullTree();
	~CullTree() = default;

	void updateTree();
	void destroyNode(int32_t nodeId);
	void setScene(Scene *scene);
	void setRenderEnable(int32_t nodeId);
	void setRenderDisable(int32_t nodeId);
	void frustumCulling(const Frustum &frustum, int32_t nodeId);
	void changeEntityHandle(int32_t nodeId, uint32_t entityHandle);
	int32_t createNode(const CullSphere &sphere, uint32_t entityHandle);

	int32_t getRootNodeId();

	void printCullTree(int32_t nodeId);
	int32_t getSize();

	// const AABB &getFatAABB(int32_t proxyId) const;

  private:
	// 가장 최적의 위치를 찾아 node 삽입, balance
	void freeNode(int32_t nodeId);
	void insertLeaf(int32_t leaf);
	void detachNode(int32_t nodeId);
	bool moveNode(int32_t nodeId, const CullSphere &newSphere);
	float getInsertionCost(const CullSphere &leafSphere, int32_t child, float inheritedCost);
	float getInsertionCostForLeaf(const CullSphere &leafSphere, int32_t child, float inheritedCost);
	int32_t balance(int32_t index);
	int32_t allocateNode();

	Scene *m_scene;
	int32_t m_root;
	int32_t m_freeNode;
	int32_t m_nodeCount;
	int32_t m_nodeCapacity;
	std::vector<CullTreeNode> m_nodes;
};

} // namespace ale
