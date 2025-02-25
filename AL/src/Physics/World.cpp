#include "Physics/World.h"
#include "Physics/Rigidbody.h"
#include "Physics/Shape/BoxShape.h"
#include "Physics/Shape/SphereShape.h"

namespace ale
{
World::World() : m_rigidbodies(nullptr), m_rigidbodyCount(0) {};

World::~World()
{
	Rigidbody *body = m_rigidbodies;

	while (body != nullptr)
	{
		Rigidbody *nextBody = body->next;
		body->~Rigidbody();
		PhysicsAllocator::m_blockAllocator.freeBlock(body, sizeof(Rigidbody));

		body = nextBody;
	}
}

void World::startFrame()
{
	Rigidbody *body = m_rigidbodies;
	while (body != nullptr)
	{
		body->clearAccumulators();
		body->calculateDerivedData();
		body = body->next;
	}
}

void World::runPhysics(float duration)
{
	Rigidbody *body = m_rigidbodies;
	while (body != nullptr)
	{
		if (body->isAwake())
		{
			body->calculateForceAccum();
			body->integrate(duration);
			body->synchronizeFixtures();
		}
		body = body->next;
	}
	m_contactManager.findNewContacts();
	m_contactManager.collide();
	solve(duration);
}

void World::solve(float duration)
{
	// island 초기화
	Island island(m_rigidbodyCount, m_contactManager.m_contactCount);

	// 모든 body들의 플래그에 islandFlag 제거
	for (Rigidbody *body = m_rigidbodies; body; body = body->next)
	{
		body->unsetFlag(EBodyFlag::ISLAND);
	}

	// 모든 contact들의 플래그에 islandFlag 제거
	for (Contact *contact = m_contactManager.m_contactList; contact; contact = contact->getNext())
	{
		contact->unsetFlag(EContactFlag::ISLAND);
	}

	// Body를 순회하며 island를 생성후 solve 처리
	Rigidbody **stack = static_cast<Rigidbody **>(PhysicsAllocator::m_stackAllocator.allocateStack(m_rigidbodyCount));
	int32_t stackPtr = 0;

	// body 순회
	for (Rigidbody *body = m_rigidbodies; body; body = body->next)
	{
		// 이미 island에 포함된 경우 continue
		if (body->hasFlag(EBodyFlag::ISLAND))
		{
			continue;
		}

		// staticBody인 경우 continue
		if (body->getType() == EBodyType::STATIC_BODY)
		{
			continue;
		}

		// 현재 Body가 island 생성 가능하다 판단이 끝났으니
		// island clear를 통해 새로운 island 생성
		island.clear();
		stack[stackPtr] = body;
		++stackPtr;
		body->setFlag(EBodyFlag::ISLAND); // body island 처리

		// DFS로 island 생성
		while (stackPtr > 0)
		{
			// 스택 가장 마지막에 있는 body island에 추가
			Rigidbody *targetBody = stack[--stackPtr];
			island.add(targetBody);

			// body가 staticBody면 뒤에 과정 pass
			if (targetBody->getType() == EBodyType::STATIC_BODY)
			{
				continue;
			}

			// body contactList의 contact들을 island에 추가
			for (ContactLink *link = targetBody->getContactLinks(); link; link = link->next)
			{
				Contact *contact = link->contact;

				// 이미 island에 포함된 경우 continue
				if (contact->hasFlag(EContactFlag::ISLAND))
				{
					continue;
				}

				// contact가 touching 상태가 아니면 continue
				if (contact->hasFlag(EContactFlag::TOUCHING) == false)
				{
					continue;
				}

				// 위 조건을 다 충족하는 경우 island에 추가 후 island 플래그 on
				island.add(contact);
				contact->setFlag(EContactFlag::ISLAND);

				Rigidbody *other = link->other;

				// 충돌 상대 body가 이미 island에 속한 상태면 continue
				if (other->hasFlag(EBodyFlag::ISLAND))
				{
					continue;
				}

				// 충돌 상대 body가 island에 속한게 아니었으면 stack에 추가 후 island 플래그 on
				stack[stackPtr] = other;
				stackPtr++;
				other->setFlag(EBodyFlag::ISLAND);
			}
		}

		// 생성한 island 충돌 처리
		island.solve(duration);

		// island의 staticBody들의 island 플래그 off
		Rigidbody **islandBodies = island.m_bodies;
		for (int32_t i = 0; i < island.m_bodyCount; ++i)
		{

			if (islandBodies[i]->getType() == EBodyType::STATIC_BODY)
			{
				islandBodies[i]->unsetFlag(EBodyFlag::ISLAND);
			}
		}
	}

	island.destroy();

	PhysicsAllocator::m_stackAllocator.freeStack();
}

Rigidbody *World::createBody(BodyDef &bdDef)
{
	void *bodyMemory = PhysicsAllocator::m_blockAllocator.allocateBlock(sizeof(Rigidbody));
	Rigidbody *body = new (static_cast<Rigidbody *>(bodyMemory)) Rigidbody(&bdDef, this);

	if (m_rigidbodyCount != 0)
	{
		m_rigidbodies->prev = body;
	}

	body->prev = nullptr;
	body->next = m_rigidbodies;
	m_rigidbodies = body;
	++m_rigidbodyCount;

	return body;
}

void World::registerBodyForce(int32_t idx, const alglm::vec3 &force)
{
	// check idx
	Rigidbody *body = m_rigidbodies;
	for (int32_t i = m_rigidbodyCount - 1; i > idx; --i)
	{
		body = body->next;
	}
	body->registerForce(force);
}

} // namespace ale