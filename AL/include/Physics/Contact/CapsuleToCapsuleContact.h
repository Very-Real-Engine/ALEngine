#pragma once

#include "Physics/Contact/Contact.h"

namespace ale
{
class CapsuleToCapsuleContact : public Contact
{
  public:
	static Contact *create(Fixture *fixtureA, Fixture *fixtureB, int32_t indexA, int32_t indexB);
	CapsuleToCapsuleContact(Fixture *fixtureA, Fixture *fixtureB, int32_t indexA, int32_t indexB);

	virtual alglm::vec3 supportA(const ConvexInfo &capsule, alglm::vec3 dir) override;
	virtual alglm::vec3 supportB(const ConvexInfo &capsule, alglm::vec3 dir) override;
	virtual void findCollisionPoints(const ConvexInfo &capsuleA, const ConvexInfo &capsuleB,
									 CollisionInfo &collisionInfo, EpaInfo &epaInfo,
									 SimplexArray &simplexArray) override;
};
} // namespace ale