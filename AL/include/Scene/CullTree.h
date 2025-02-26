#pragma once

#include "Core/Log.h"
#include "Renderer/Common.h"

namespace ale
{

#define NULL_NODE (-1)

/**
 * @file
 * @brief 카메라 뷰 프러스텀(Frustum) 컬링 및 트리 구조를 관리하는 CullTree 클래스 정의.
 */

/**
 * @enum ECullState
 * @brief 컬링 상태를 정의하는 열거형.
 */
enum class ECullState
{
	CULL = 0,
	RENDER = (1 << 0),
	NONE = (1 << 1),
	RENDER_AND_NONE = (1 << 0) | (1 << 1),
};

/** @brief ECullState의 AND 연산자 오버로딩. */
ECullState operator&(ECullState state1, ECullState state2);

/** @brief ECullState의 OR 연산자 오버로딩. */
ECullState operator|(ECullState state1, ECullState state2);

/**
 * @struct CullSphere
 * @brief 컬링을 위한 경계 구(Sphere) 구조체.
 */
struct CullSphere
{
	alglm::vec3 center;
	float radius;

	/** @brief 기본 생성자. */
	CullSphere() = default;

	/** @brief 중심 좌표 및 반지름을 설정하는 생성자. */
	CullSphere(alglm::vec3 &center, float radius) : center(center), radius(radius) {};

	/** @brief 중심 좌표(4D) 및 반지름을 설정하는 생성자. */
	CullSphere(alglm::vec4 &center, float radius) : center(center), radius(radius) {};

	/** @brief 구의 부피를 계산합니다. */
	float getVolume()
	{
		return radius * radius * radius;
	}

	/**
	 * @brief 두 개의 구를 합쳐 새로운 구를 생성합니다.
	 * @param sphere1 첫 번째 구.
	 * @param sphere2 두 번째 구.
	 */
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

/**
 * @struct FrustumPlane
 * @brief 프러스텀의 평면 정보를 저장하는 구조체.
 */
struct FrustumPlane
{
	float distance;
	alglm::vec3 normal;

	/** @brief 기본 생성자. */
	FrustumPlane() = default;

	/**
	 * @brief 3개의 점을 사용하여 프러스텀 평면을 생성하는 생성자.
	 * @param p1 첫 번째 점.
	 * @param p2 두 번째 점.
	 * @param p3 세 번째 점.
	 */
	FrustumPlane(const alglm::vec3 &p1, const alglm::vec3 &p2, const alglm::vec3 &p3)
	{
		normal = alglm::normalize(alglm::cross(p2 - p1, p3 - p1));
		distance = alglm::dot(normal, p1);
	}
};

/**
 * @enum EFrustum
 * @brief 프러스텀 컬링 결과를 나타내는 열거형.
 */
enum class EFrustum
{
	OUTSIDE,
	INSIDE,
	INTERSECT,
};

/**
 * @struct Frustum
 * @brief 카메라의 뷰 프러스텀을 정의하는 구조체.
 */
struct Frustum
{
	// 0: near, 1: far, 2: left, 3: right, 4: up, 5: down
	FrustumPlane plane[6];

	/**
	 * @brief 구(Sphere)가 프러스텀 내부에 있는지 검사합니다.
	 * @param sphere 검사할 CullSphere.
	 * @return 컬링 결과 (INSIDE, OUTSIDE, INTERSECT).
	 */
	EFrustum cullingSphere(const CullSphere &sphere) const
	{
		bool intersect = false;

		for (int32_t i = 0; i < 6; ++i)
		{
			float dotResult = alglm::dot(plane[i].normal, sphere.center);
			float distance = plane[i].distance;
			if (dotResult - sphere.radius > distance)
			{
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

/**
 * @struct CullTreeNode
 * @brief 컬링 트리의 노드를 정의하는 구조체.
 */
struct CullTreeNode
{
	/**
	 * @brief 노드가 리프(Leaf)인지 확인합니다.
	 * @return 리프 노드이면 true, 그렇지 않으면 false.
	 */
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

/**
 * @class CullTree
 * @brief 씬(Scene) 내에서 컬링을 수행하는 트리 구조를 관리하는 클래스.
 */
class CullTree
{
  public:
	/** @brief CullTree 기본 생성자. */
	CullTree();

	/** @brief CullTree 소멸자. */
	~CullTree() = default;

	/** @brief 컬링 트리를 업데이트합니다. */
	void updateTree();

	/** @brief 특정 노드를 삭제합니다. */
	void destroyNode(int32_t nodeId);

	/** @brief 컬링 트리에 씬(Scene)을 설정합니다. */
	void setScene(Scene *scene);

	/** @brief 특정 노드를 렌더링 가능 상태로 설정합니다. */
	void setRenderEnable(int32_t nodeId);

	/** @brief 특정 노드를 렌더링 불가능 상태로 설정합니다. */
	void setRenderDisable(int32_t nodeId);

	/** @brief 프러스텀 컬링을 수행합니다. */
	void frustumCulling(const Frustum &frustum, int32_t nodeId);

	/** @brief 특정 노드의 엔터티 핸들을 변경합니다. */
	void changeEntityHandle(int32_t nodeId, uint32_t entityHandle);

	/** @brief 새로운 노드를 생성하고 트리에 추가합니다. */
	int32_t createNode(const CullSphere &sphere, uint32_t entityHandle);

	/** @brief 루트 노드의 ID를 반환합니다. */
	int32_t getRootNodeId();

	/** @brief CullTree의 구조를 출력합니다. */
	void printCullTree(int32_t nodeId);

	/** @brief CullTree 내 노드 개수를 반환합니다. */
	int32_t getSize();

	// const AABB &getFatAABB(int32_t proxyId) const;

  private:
	/** @brief 특정 노드를 해제합니다. */
	void freeNode(int32_t nodeId);

	/** @brief 새로운 리프 노드를 트리에 삽입합니다. */
	void insertLeaf(int32_t leaf);

	/** @brief 특정 노드를 트리에서 분리합니다. */
	void detachNode(int32_t nodeId);

	/** @brief 노드를 새로운 위치로 이동합니다. */
	bool moveNode(int32_t nodeId, const CullSphere &newSphere);

	/** @brief 노드 삽입 비용을 계산합니다. */
	float getInsertionCost(const CullSphere &leafSphere, int32_t child, float inheritedCost);

	/** @brief 리프 노드의 삽입 비용을 계산합니다. */
	float getInsertionCostForLeaf(const CullSphere &leafSphere, int32_t child, float inheritedCost);

	/** @brief 트리 균형을 맞춥니다. */
	int32_t balance(int32_t index);

	/** @brief 새로운 노드를 할당합니다. */
	int32_t allocateNode();

	Scene *m_scene;
	int32_t m_root;
	int32_t m_freeNode;
	int32_t m_nodeCount;
	int32_t m_nodeCapacity;
	std::vector<CullTreeNode> m_nodes;
};

} // namespace ale
