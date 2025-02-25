#include "Physics/Island.h"
#include "Physics/Contact/ContactSolver.h"

namespace ale
{
const int32_t Island::VELOCITY_ITERATION = 10;
const int32_t Island::POSITION_ITERATION = 10;
const float Island::STOP_LINEAR_VELOCITY = 1.0f;
const float Island::STOP_ANGULAR_VELOCITY = 0.1f;

Island::Island(int32_t bodyCount, int32_t contactCount)
{
	m_bodyCount = 0;
	m_contactCount = 0;
	m_bodies =
		static_cast<Rigidbody **>(PhysicsAllocator::m_stackAllocator.allocateStack(sizeof(Rigidbody *) * bodyCount));
	m_contacts =
		static_cast<Contact **>(PhysicsAllocator::m_stackAllocator.allocateStack(sizeof(Contact *) * contactCount));
}

void Island::destroy()
{
	PhysicsAllocator::m_stackAllocator.freeStack();
	PhysicsAllocator::m_stackAllocator.freeStack();
}

void Island::solve(float duration)
{
	if (m_bodyCount == 1)
	{
		return;
	}
	m_positions =
		static_cast<Position *>(PhysicsAllocator::m_stackAllocator.allocateStack(sizeof(Position) * m_bodyCount));
	m_velocities =
		static_cast<Velocity *>(PhysicsAllocator::m_stackAllocator.allocateStack(sizeof(Velocity) * m_bodyCount));

	// 힘을 적용하여 속도, 위치, 회전 업데이트
	for (int32_t i = 0; i < m_bodyCount; i++)
	{
		Rigidbody *body = m_bodies[i];

		new (m_positions + i) Position();
		new (m_velocities + i) Velocity();

		// 위치, 각도, 속도 기록
		m_positions[i].position = body->getPosition();
		m_velocities[i].linearVelocity = body->getLinearVelocity();
		m_velocities[i].angularVelocity = body->getAngularVelocity();

		m_positions[i].positionBuffer = alglm::vec3(0.0f);
		m_velocities[i].linearVelocityBuffer = alglm::vec3(0.0f);
		m_velocities[i].angularVelocityBuffer = alglm::vec3(0.0f);
	}

	ContactSolver contactSolver(duration, m_contacts, m_positions, m_velocities, m_bodyCount, m_contactCount);

	// 속도 제약 반복 횟수만큼 반복
	for (int32_t i = 0; i < VELOCITY_ITERATION; ++i)
	{
		// 충돌 속도 제약 해결
		contactSolver.solveVelocityConstraints();
	}

	// 위치 제약 처리 반복
	for (int32_t i = 0; i < POSITION_ITERATION; ++i)
	{
		contactSolver.solvePositionConstraints();
	}

	contactSolver.checkSleepContact();

	// 위치, 회전, 속도 업데이트
	for (int32_t i = 0; i < m_bodyCount; ++i)
	{

		Rigidbody *body = m_bodies[i];
		if (body->getType() == EBodyType::STATIC_BODY)
		{
			continue;
		}

		if (m_positions[i].isNormalStop && m_positions[i].isTangentStop && m_positions[i].isNormal &&
			alglm::length(m_velocities[i].linearVelocity) < STOP_LINEAR_VELOCITY &&
			alglm::length(m_velocities[i].angularVelocity) < STOP_ANGULAR_VELOCITY)
		{
			m_velocities[i].linearVelocity = alglm::vec3(0.0f);
			m_velocities[i].angularVelocity = alglm::vec3(0.0f);
			body->setSleep(duration);
		}
		else
		{
			body->setAwake();
		}

		body->updateSweep();
		body->setPosition(m_positions[i].position + m_positions[i].positionBuffer);
		body->setLinearVelocity(m_velocities[i].linearVelocity);
		body->setAngularVelocity(m_velocities[i].angularVelocity);
		body->synchronizeFixtures();
	}

	contactSolver.destroy();

	for (int32_t i = 0; i < m_bodyCount; ++i)
	{
		m_positions[i].~Position();
		m_velocities[i].~Velocity();
	}

	PhysicsAllocator::m_stackAllocator.freeStack();
	PhysicsAllocator::m_stackAllocator.freeStack();
}

void Island::add(Rigidbody *body)
{
	body->setIslandIndex(m_bodyCount);
	m_bodies[m_bodyCount] = body;
	++m_bodyCount;
}

void Island::add(Contact *contact)
{
	m_contacts[m_contactCount] = contact;
	++m_contactCount;
}

void Island::clear()
{
	memset(m_bodies, 0, sizeof(Rigidbody *) * m_bodyCount);
	memset(m_contacts, 0, sizeof(Contact *) * m_contactCount);
	m_bodyCount = 0;
	m_contactCount = 0;
}
} // namespace ale