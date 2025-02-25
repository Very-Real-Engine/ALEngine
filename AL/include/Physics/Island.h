#pragma once

#include "Physics/Contact/Contact.h"

namespace ale
{

struct Position
{
	alglm::vec3 position;
	alglm::vec3 positionBuffer;
	bool isNormalStop{true};
	bool isTangentStop{true};
	bool isNormal{false};
};

struct Velocity
{
	alglm::vec3 linearVelocity;
	alglm::vec3 angularVelocity;
	alglm::vec3 linearVelocityBuffer;
	alglm::vec3 angularVelocityBuffer;
};

class Island
{
  public:
	Island(int32_t bodyCount, int32_t contactCount);
	void solve(float duration);
	void destroy();

	void add(Rigidbody *body);
	void add(Contact *contact);
	void clear();

	static const int32_t VELOCITY_ITERATION;
	static const int32_t POSITION_ITERATION;
	static const float STOP_LINEAR_VELOCITY;
	static const float STOP_ANGULAR_VELOCITY;

	Rigidbody **m_bodies;
	Contact **m_contacts;
	Position *m_positions;
	Velocity *m_velocities;

	int32_t m_bodyCount;
	int32_t m_contactCount;
};

} // namespace ale
