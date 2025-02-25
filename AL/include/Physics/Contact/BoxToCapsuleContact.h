#pragma once

#include "Physics/Contact/Contact.h"

namespace ale
{
class BoxToCapsuleContact : public Contact
{
  public:
	static Contact *create(Fixture *fixtureA, Fixture *fixtureB, int32_t indexA, int32_t indexB);
	BoxToCapsuleContact(Fixture *fixtureA, Fixture *fixtureB, int32_t indexA, int32_t indexB);

	virtual alglm::vec3 supportA(const ConvexInfo &box, alglm::vec3 dir) override;
	virtual alglm::vec3 supportB(const ConvexInfo &capsule, alglm::vec3 dir) override;
	virtual void findCollisionPoints(const ConvexInfo &box, const ConvexInfo &capsule, CollisionInfo &collisionInfo,
									 EpaInfo &epaInfo, SimplexArray &simplexArray) override;
};
} // namespace ale