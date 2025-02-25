#pragma once

#include "Physics/Island.h"
#include "Physics/PhysicsAllocator.h"

namespace ale
{

struct ContactPositionConstraint
{
	ManifoldPoint *points;
	int32_t pointCount;
	alglm::vec3 worldCenterA;
	alglm::vec3 worldCenterB;
	alglm::mat3 invIA, invIB;
	int32_t indexA;
	int32_t indexB;
	float invMassA, invMassB;

	~ContactPositionConstraint() = default;
};

struct ContactVelocityConstraint
{
	ManifoldPoint *points;
	int32_t pointCount;
	alglm::vec3 worldCenterA;
	alglm::vec3 worldCenterB;
	alglm::mat3 invIA, invIB;
	int32_t indexA, indexB;
	float invMassA, invMassB;
	float friction;
	float restitution;

	~ContactVelocityConstraint() = default;
};

class ContactSolver
{
  public:
	ContactSolver(float duration, Contact **contacts, Position *positions, Velocity *velocities, int32_t bodyCount,
				  int32_t contactCount);
	void destroy();
	void initializeVelocityConstraints();
	void solveVelocityConstraints();
	void solvePositionConstraints();
	void checkSleepContact();

	static const float NORMAL_STOP_VELOCITY;
	static const float TANGENT_STOP_VELOCITY;
	static const float NORMAL_SLEEP_VELOCITY;
	static const float TANGENT_SLEEP_VELOCITY;

	int32_t m_bodyCount;
	int32_t m_contactCount;
	float m_duration;
	Contact **m_contacts;
	Position *m_positions;
	Velocity *m_velocities;
	ContactPositionConstraint *m_positionConstraints;
	ContactVelocityConstraint *m_velocityConstraints;
};

} // namespace ale