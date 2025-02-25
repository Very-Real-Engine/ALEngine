#include "Physics/Contact/ContactSolver.h"

namespace ale
{

const float ContactSolver::NORMAL_STOP_VELOCITY = 0.0001f;
const float ContactSolver::TANGENT_STOP_VELOCITY = 0.0001f;
const float ContactSolver::NORMAL_SLEEP_VELOCITY = 1.0f;
const float ContactSolver::TANGENT_SLEEP_VELOCITY = 1.0f;

ContactSolver::ContactSolver(float duration, Contact **contacts, Position *positions, Velocity *velocities,
							 int32_t bodyCount, int32_t contactCount)
	: m_duration(duration), m_positions(positions), m_velocities(velocities), m_contacts(contacts),
	  m_bodyCount(bodyCount), m_contactCount(contactCount)
{
	m_positionConstraints = static_cast<ContactPositionConstraint *>(
		PhysicsAllocator::m_stackAllocator.allocateStack(sizeof(ContactPositionConstraint) * contactCount));
	m_velocityConstraints = static_cast<ContactVelocityConstraint *>(
		PhysicsAllocator::m_stackAllocator.allocateStack(sizeof(ContactVelocityConstraint) * contactCount));

	for (int32_t i = 0; i < contactCount; i++)
	{
		Contact *contact = m_contacts[i];

		// contact 정보 가져오기
		Fixture *fixtureA = contact->getFixtureA();
		Fixture *fixtureB = contact->getFixtureB();
		Shape *shapeA = fixtureA->getShape();
		Shape *shapeB = fixtureB->getShape();
		Rigidbody *bodyA = fixtureA->getBody();
		Rigidbody *bodyB = fixtureB->getBody();
		Manifold &manifold = contact->getManifold();

		new (m_velocityConstraints + i) ContactVelocityConstraint();
		new (m_positionConstraints + i) ContactPositionConstraint();

		// 속도 제약 설정
		m_velocityConstraints[i].friction = contact->getFriction();
		m_velocityConstraints[i].restitution = contact->getRestitution();
		m_velocityConstraints[i].worldCenterA = bodyA->getTransform().toMatrix() * alglm::vec4(shapeA->m_center, 1.0f);
		m_velocityConstraints[i].worldCenterB = bodyB->getTransform().toMatrix() * alglm::vec4(shapeB->m_center, 1.0f);
		m_velocityConstraints[i].indexA = bodyA->getIslandIndex();
		m_velocityConstraints[i].indexB = bodyB->getIslandIndex();
		m_velocityConstraints[i].invMassA = bodyA->getInverseMass();
		m_velocityConstraints[i].invMassB = bodyB->getInverseMass();
		m_velocityConstraints[i].invIA = bodyA->getInverseInertiaTensorWorld();
		m_velocityConstraints[i].invIB = bodyB->getInverseInertiaTensorWorld();
		m_velocityConstraints[i].pointCount = manifold.pointsCount;
		m_velocityConstraints[i].points = manifold.points;

		// 위치 제약 설정
		m_positionConstraints[i].worldCenterA = bodyA->getTransform().toMatrix() * alglm::vec4(shapeA->m_center, 1.0f);
		m_positionConstraints[i].worldCenterB = bodyB->getTransform().toMatrix() * alglm::vec4(shapeB->m_center, 1.0f);
		m_positionConstraints[i].indexA = bodyA->getIslandIndex();
		m_positionConstraints[i].indexB = bodyB->getIslandIndex();
		m_positionConstraints[i].invMassA = bodyA->getInverseMass();
		m_positionConstraints[i].invMassB = bodyB->getInverseMass();
		m_velocityConstraints[i].invIA = bodyA->getInverseInertiaTensorWorld();
		m_velocityConstraints[i].invIB = bodyB->getInverseInertiaTensorWorld();
		m_positionConstraints[i].pointCount = manifold.pointsCount;
		m_positionConstraints[i].points = manifold.points;
	}
}

void ContactSolver::destroy()
{
	for (int32_t i = 0; i < m_contactCount; i++)
	{
		m_positionConstraints[i].~ContactPositionConstraint();
		m_velocityConstraints[i].~ContactVelocityConstraint();
	}

	PhysicsAllocator::m_stackAllocator.freeStack();
	PhysicsAllocator::m_stackAllocator.freeStack();
}

void ContactSolver::solveVelocityConstraints()
{
	for (int32_t i = 0; i < m_contactCount; i++)
	{
		Contact *contact = m_contacts[i];
		ContactVelocityConstraint &velocityConstraint = m_velocityConstraints[i];
		int32_t pointCount = velocityConstraint.pointCount;
		int32_t indexA = velocityConstraint.indexA;
		int32_t indexB = velocityConstraint.indexB;

		alglm::vec3 &linearVelocityA = m_velocities[indexA].linearVelocity;
		alglm::vec3 &linearVelocityB = m_velocities[indexB].linearVelocity;
		alglm::vec3 &angularVelocityA = m_velocities[indexA].angularVelocity;
		alglm::vec3 &angularVelocityB = m_velocities[indexB].angularVelocity;

		alglm::vec3 &linearVelocityBufferA = m_velocities[indexA].linearVelocityBuffer;
		alglm::vec3 &linearVelocityBufferB = m_velocities[indexB].linearVelocityBuffer;
		alglm::vec3 &angularVelocityBufferA = m_velocities[indexA].angularVelocityBuffer;
		alglm::vec3 &angularVelocityBufferB = m_velocities[indexB].angularVelocityBuffer;

		bool isStop = true;

		float seperationSum = 0.0f;

		for (int32_t j = 0; j < pointCount; ++j)
		{
			seperationSum += velocityConstraint.points[j].seperation;
		}

		if (seperationSum <= 0.0f)
		{
			continue;
		}

		for (int32_t j = 0; j < pointCount; ++j)
		{
			ManifoldPoint &manifoldPoint = velocityConstraint.points[j];

			alglm::vec3 rA = manifoldPoint.pointA - velocityConstraint.worldCenterA; // bodyA의 충돌 지점까지의 벡터
			alglm::vec3 rB = manifoldPoint.pointB - velocityConstraint.worldCenterB; // bodyB의 충돌 지점까지의 벡터

			// 상대 속도 계산
			alglm::vec3 velocityA = linearVelocityA + alglm::cross(angularVelocityA, rA);
			alglm::vec3 velocityB = linearVelocityB + alglm::cross(angularVelocityB, rB);

			// rA, rB 중 1개라도 0.0f 면 continue (cross 연산 오류 발생 가능)
			if (alglm::length2(rA) == 0.0f || alglm::length2(rB) == 0.0f)
			{
				continue;
			}

			// 상대 속도
			alglm::vec3 relativeVelocity = velocityB - velocityA;

			// 법선 방향 속도
			float normalSpeed = alglm::dot(relativeVelocity, manifoldPoint.normal);
			float appliedNormalImpulse;

			if (normalSpeed < -NORMAL_STOP_VELOCITY)
			{
				// std::cout << "normal in!!\n";

				float oldNormalImpulse = manifoldPoint.normalImpulse;
				appliedNormalImpulse =
					-(1.0f + velocityConstraint.restitution) * normalSpeed * (manifoldPoint.seperation / seperationSum);
				float inverseMasses = (velocityConstraint.invMassA + velocityConstraint.invMassB);
				float normalEffectiveMassA =
					alglm::dot(alglm::cross(manifoldPoint.normal, rA),
							   velocityConstraint.invIA * alglm::cross(manifoldPoint.normal, rA));
				float normalEffectiveMassB =
					alglm::dot(alglm::cross(manifoldPoint.normal, rB),
							   velocityConstraint.invIB * alglm::cross(manifoldPoint.normal, rB));

				appliedNormalImpulse =
					appliedNormalImpulse / (inverseMasses + normalEffectiveMassA + normalEffectiveMassB);

				float newNormalImpulse = appliedNormalImpulse + oldNormalImpulse;

				manifoldPoint.normalImpulse = newNormalImpulse;

				if (manifoldPoint.normalImpulse <= 0.0f)
				{
					manifoldPoint.normalImpulse = 0.0f;
				}

				linearVelocityBufferA -= velocityConstraint.invMassA * appliedNormalImpulse * manifoldPoint.normal;
				linearVelocityBufferB += velocityConstraint.invMassB * appliedNormalImpulse * manifoldPoint.normal;

				angularVelocityBufferA -=
					velocityConstraint.invIA * alglm::cross(rA, appliedNormalImpulse * manifoldPoint.normal);
				angularVelocityBufferB +=
					velocityConstraint.invIB * alglm::cross(rB, appliedNormalImpulse * manifoldPoint.normal);
			}

			// 접선 방향 충격량 계산
			alglm::vec3 tangentVelocity = relativeVelocity - (normalSpeed * manifoldPoint.normal);
			alglm::vec3 tangent = alglm::normalize(tangentVelocity);
			float tangentSpeed = alglm::dot(tangent, tangentVelocity);

			if (tangentSpeed > TANGENT_STOP_VELOCITY && alglm::length2(tangent) != 0.0f)
			{
				float oldTangentImpulse = manifoldPoint.tangentImpulse;
				float newTangentImpulse = tangentSpeed * (manifoldPoint.seperation / seperationSum);

				float inverseMasses = (velocityConstraint.invMassA + velocityConstraint.invMassB);

				float tangentEffectiveMassA =
					alglm::dot(alglm::cross(tangent, rA), velocityConstraint.invIA * alglm::cross(tangent, rA));
				float tangentEffectiveMassB =
					alglm::dot(alglm::cross(tangent, rB), velocityConstraint.invIB * alglm::cross(tangent, rB));
				newTangentImpulse = newTangentImpulse / (inverseMasses + tangentEffectiveMassA + tangentEffectiveMassB);
				newTangentImpulse += oldTangentImpulse;

				float maxFriction = velocityConstraint.friction * manifoldPoint.normalImpulse;
				newTangentImpulse = alglm::clamp(newTangentImpulse, -maxFriction, maxFriction);

				manifoldPoint.tangentImpulse = newTangentImpulse;

				float appliedTangentImpulse = newTangentImpulse - oldTangentImpulse;

				linearVelocityBufferA += velocityConstraint.invMassA * appliedTangentImpulse * tangent;
				linearVelocityBufferB -= velocityConstraint.invMassB * appliedTangentImpulse * tangent;

				angularVelocityBufferA += velocityConstraint.invIA * alglm::cross(rA, appliedTangentImpulse * tangent);
				angularVelocityBufferB -= velocityConstraint.invIB * alglm::cross(rB, appliedTangentImpulse * tangent);
			}
		}
		linearVelocityA += linearVelocityBufferA;
		linearVelocityB += linearVelocityBufferB;
		angularVelocityA += angularVelocityBufferA;
		angularVelocityB += angularVelocityBufferB;
		linearVelocityBufferA = alglm::vec3(0.0f);
		linearVelocityBufferB = alglm::vec3(0.0f);
		angularVelocityBufferA = alglm::vec3(0.0f);
		angularVelocityBufferB = alglm::vec3(0.0f);
	}
}

void ContactSolver::solvePositionConstraints()
{
	const float kSlop = 0.001f; // 허용 관통 오차
	const float alpha = 1.0f;

	for (int i = 0; i < m_contactCount; ++i)
	{
		Contact *contact = m_contacts[i];
		ContactPositionConstraint &positionConstraint = m_positionConstraints[i];

		int32_t pointCount = positionConstraint.pointCount;
		int32_t indexA = positionConstraint.indexA;
		int32_t indexB = positionConstraint.indexB;

		alglm::mat3 &invIA = positionConstraint.invIA;
		alglm::mat3 &invIB = positionConstraint.invIB;

		float sumMass = positionConstraint.invMassA + positionConstraint.invMassB;
		float ratioA = positionConstraint.invMassA / sumMass;
		float ratioB = positionConstraint.invMassB / sumMass;

		alglm::vec3 &positionBufferA = m_positions[indexA].positionBuffer;
		alglm::vec3 &positionBufferB = m_positions[indexB].positionBuffer;

		for (int32_t j = 0; j < pointCount; j++)
		{

			ManifoldPoint &manifoldPoint = positionConstraint.points[j];

			alglm::vec3 movedPointA = manifoldPoint.pointA + positionBufferA;
			alglm::vec3 movedPointB = manifoldPoint.pointB + positionBufferB;

			float seperation = alglm::dot(manifoldPoint.normal, movedPointA - movedPointB);

			// 관통 해소된상태면 무시
			if (seperation < kSlop)
			{
				continue;
			}

			// 관통 깊이에 따른 보정량 계산
			float correction = seperation * alpha / pointCount;
			alglm::vec3 correctionVector = correction * manifoldPoint.normal;

			positionBufferA -= correctionVector * ratioA;
			positionBufferB += correctionVector * ratioB;
		}
	}
}

void ContactSolver::checkSleepContact()
{

	for (int32_t i = 0; i < m_contactCount; i++)
	{
		Contact *contact = m_contacts[i];
		ContactVelocityConstraint &velocityConstraint = m_velocityConstraints[i];
		int32_t pointCount = velocityConstraint.pointCount;
		int32_t indexA = velocityConstraint.indexA;
		int32_t indexB = velocityConstraint.indexB;

		alglm::vec3 &linearVelocityA = m_velocities[indexA].linearVelocity;
		alglm::vec3 &linearVelocityB = m_velocities[indexB].linearVelocity;
		alglm::vec3 &angularVelocityA = m_velocities[indexA].angularVelocity;
		alglm::vec3 &angularVelocityB = m_velocities[indexB].angularVelocity;
		alglm::vec3 upVector = alglm::vec3(0.0f, 1.0f, 0.0f);

		for (int32_t j = 0; j < pointCount; ++j)
		{
			ManifoldPoint &manifoldPoint = velocityConstraint.points[j];

			alglm::vec3 relativeVelocity = linearVelocityA - linearVelocityB;

			if (alglm::length(relativeVelocity) > 1.0f)
			{
				m_positions[indexA].isNormalStop = false;
				m_positions[indexB].isNormalStop = false;
			}

			alglm::vec3 relativeAngularVelocity = angularVelocityA - angularVelocityB;
			if (alglm::length(relativeAngularVelocity) > 1.0f)
			{
				m_positions[indexA].isTangentStop = false;
				m_positions[indexB].isTangentStop = false;
			}

			float normalDotUpVector = alglm::dot(manifoldPoint.normal, upVector);
			if (normalDotUpVector < -0.3f)
			{
				m_positions[indexA].isNormal = true;
			}
			if (normalDotUpVector > 0.3f)
			{
				m_positions[indexB].isNormal = true;
			}
		}
	}
}
} // namespace ale
