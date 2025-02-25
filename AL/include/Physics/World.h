#pragma once

#include "Physics/Contact/ContactManager.h"
#include "Physics/Island.h"

#include <stack>

class Model;
class App;

namespace ale
{
class Rigidbody;
class ContactManager;
class BoxShape;
class SphereShape;

class World
{
  public:
	World();
	~World();

	void startFrame();
	void runPhysics(float duration);
	void solve(float duration);
	void registerBodyForce(int32_t idx, const alglm::vec3 &force);

	Rigidbody *createBody(BodyDef &bdDef);
	Rigidbody *getBodyList()
	{
		return m_rigidbodies;
	}

	ContactManager m_contactManager;

  private:
	Rigidbody *m_rigidbodies;
	int32_t m_rigidbodyCount;
};
} // namespace ale