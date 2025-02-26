#pragma once

#include "Physics/DynamicTree.h"

#include <set>
#include <utility>

namespace ale
{
/**
 * @file
 * @brief BroadPhase 충돌 감지를 수행하는 BroadPhase 클래스 정의.
 */

class DynamicTree;

/**
 * @class BroadPhase
 * @brief BroadPhase 충돌 감지를 담당하는 클래스.
 * @details BroadPhase는 DynamicTree를 사용하여 AABB(축 정렬 바운딩 박스) 기반의 충돌 감지를 수행하며,
 *         빠른 충돌 후보군을 찾는 역할을 합니다.
 */
class BroadPhase
{
  public:
	/** @brief 널(무효) 프록시 ID를 나타내는 상수. */
	enum
	{
		NULL_PROXY = -1
	};

	/**
	 * @brief BroadPhase 기본 생성자.
	 */
	BroadPhase();

	/**
	 * @brief AABB를 기반으로 Proxy를 생성합니다.
	 * @param aabb Proxy를 생성할 AABB 영역.
	 * @param userData 사용자 데이터 포인터.
	 * @return 생성된 Proxy의 ID (nodeId).
	 */
	int32_t createProxy(const AABB &aabb, void *userData);

	/**
	 * @brief Proxy를 제거합니다.
	 * @param proxyId 제거할 Proxy의 ID.
	 */
	void destroyProxy(int32_t proxyId);

	/**
	 * @brief Proxy를 이동시킵니다.
	 * @param proxyId 이동할 Proxy의 ID.
	 * @param aabb 새로운 AABB 영역.
	 * @param displacement 이동 벡터.
	 */
	void moveProxy(int32_t proxyId, const AABB &aabb, const alglm::vec3 &displacement);

	/**
	 * @brief Proxy의 이동을 버퍼링합니다.
	 * @param proxyId 이동할 Proxy의 ID.
	 */
	void bufferMove(int32_t proxyId);

	/**
	 * @brief 충돌 후보 쌍을 갱신합니다.
	 * @tparam T 콜백 함수 타입.
	 * @param callback 충돌 후보를 처리할 콜백 함수.
	 */
	template <typename T> void updatePairs(T *callback);

	/**
	 * @brief 특정 AABB 영역과 충돌하는 후보를 탐색합니다.
	 * @tparam T 콜백 함수 타입.
	 * @param callback 검색된 충돌 후보를 처리할 콜백 함수.
	 * @param aabb 검색할 AABB 영역.
	 */
	template <typename T> void query(T *callback, const AABB &aabb) const;

  private:
	/**
	 * @brief BroadPhase의 내부 탐색을 위한 friend class 선언.
	 */
	friend class DynamicTree;

	/**
	 * @brief Query 실행 시 콜백 처리 함수.
	 * @param proxyId 검색된 Proxy ID.
	 * @return 검색이 계속 진행되면 true, 중단하면 false.
	 */
	bool queryCallback(int32_t proxyId);

	DynamicTree m_tree;
	std::set<std::pair<int32_t, int32_t>> m_proxySet;
	std::vector<int32_t> m_moveBuffer;

	int32_t m_moveCapacity;
	int32_t m_moveCount;
	int32_t m_queryProxyId;
};


template <typename T> void BroadPhase::updatePairs(T *callback)
{
	for (int32_t i = 0; i < m_moveCount; ++i)
	{
		m_queryProxyId = m_moveBuffer[i];
		if (m_queryProxyId == NULL_PROXY)
		{
			continue;
		}

		const AABB &fatAABB = m_tree.getFatAABB(m_queryProxyId);

		m_tree.query(this, fatAABB);
	}

	m_moveCount = 0;
	for (auto &it = m_proxySet.begin(); it != m_proxySet.end();)
	{
		auto primaryPair = it;
		void *userDataA = m_tree.getUserData(primaryPair->first);
		void *userDataB = m_tree.getUserData(primaryPair->second);

		callback->addPair(userDataA, userDataB);
		++it;
		while (it != m_proxySet.end())
		{
			auto pair = it;

			if (pair->first != primaryPair->first || pair->second != primaryPair->second)
			{
				break;
			}
			++it;
		}
	}
}
} // namespace ale