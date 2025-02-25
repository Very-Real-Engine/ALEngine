#include "Physics/BroadPhase.h"

namespace ale
{
BroadPhase::BroadPhase()
{
	m_moveCount = 0;
	m_moveCapacity = 16;
	m_moveBuffer.resize(m_moveCapacity);
}

int32_t BroadPhase::createProxy(const AABB &aabb, void *userData)
{
	int32_t proxyId = m_tree.createProxy(aabb, userData);
	bufferMove(proxyId);
	return proxyId;
}

void BroadPhase::destroyProxy(int32_t proxyId)
{
}

void BroadPhase::moveProxy(int32_t proxyId, const AABB &aabb, const alglm::vec3 &displacement)
{
	bool buffer = m_tree.moveProxy(proxyId, aabb, displacement);
	if (buffer)
	{
		bufferMove(proxyId);
	}
}

void BroadPhase::bufferMove(int32_t proxyId)
{
	if (m_moveCount == m_moveCapacity)
	{
		m_moveCapacity *= 2;
		m_moveBuffer.resize(m_moveCapacity);
	}
	m_moveBuffer[m_moveCount] = proxyId;
	++m_moveCount;
}

bool BroadPhase::queryCallback(int32_t proxyId)
{
	if (proxyId == m_queryProxyId)
	{
		return true;
	}

	m_proxySet.insert({std::min(proxyId, m_queryProxyId), std::max(proxyId, m_queryProxyId)});
	return true;
}
} // namespace ale