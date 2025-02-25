#pragma once

#include "Physics/Physics.h"
#include "Physics/PhysicsAllocator.h"
#include "Physics/Rigidbody.h"

namespace ale
{
class BroadPhase;
class Rigidbody;

struct FixtureDef
{
	FixtureDef()
	{
		shape = nullptr;
		userData = nullptr;
		friction = 0.0f;
		restitution = 0.0f;
		isSensor = false;
		touchNum = 0;
	}
	Shape *shape;
	void *userData;
	float friction;
	float restitution;
	bool isSensor;
	int32_t touchNum;
};

struct FixtureProxy
{
	AABB aabb;
	Fixture *fixture;
	int32_t childIndex;
	int32_t proxyId;
};

class Fixture
{
  public:
	Fixture();
	void create(Rigidbody *body, const FixtureDef *fd);
	void destroy();

	void createProxies(BroadPhase *broadPhase);
	void destroyProxies(BroadPhase *broadPhase);

	void synchronize(BroadPhase *broadPhase, const Transform &xf1, const Transform &xf2);

	float getFriction();
	float getRestitution();
	Rigidbody *getBody() const;
	EType getType() const;
	Shape *getShape();
	bool isSeonsor() const;
	int32_t getTouchNum() const ;
	void increaseTouchNum();
	void decreaseTouchNum();
	const FixtureProxy *getFixtureProxy() const;

  protected:
	Rigidbody *m_body;
	Shape *m_shape;
	float m_density;
	float m_friction;
	float m_restitution;
	bool m_isSensor;
	int32_t m_touchNum;

	FixtureProxy *m_proxies;
	int32_t m_proxyCount;
	// void *userData;

  private:
};
} // namespace ale