#pragma once

#include "Physics/Contact/Contact.h"

namespace ale
{
class SphereToCylinderContact : public Contact
{
  public:
	static Contact *create(Fixture *fixtureA, Fixture *fixtureB, int32_t indexA, int32_t indexB);
	SphereToCylinderContact(Fixture *fixtureA, Fixture *fixtureB, int32_t indexA, int32_t indexB);

	virtual alglm::vec3 supportA(const ConvexInfo &sphere, alglm::vec3 dir) override;
	virtual alglm::vec3 supportB(const ConvexInfo &cylinder, alglm::vec3 dir) override;
	virtual void findCollisionPoints(const ConvexInfo &sphere, const ConvexInfo &cylinder, CollisionInfo &collisionInfo,
									 EpaInfo &epaInfo, SimplexArray &simplexArray) override;
};
} // namespace ale