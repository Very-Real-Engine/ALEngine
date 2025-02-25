#pragma once

#include "Physics/Contact/Contact.h"

namespace ale
{
class BoxToCylinderContact : public Contact
{
  public:
	static Contact *create(Fixture *fixtureA, Fixture *fixtureB, int32_t indexA, int32_t indexB);
	BoxToCylinderContact(Fixture *fixtureA, Fixture *fixtureB, int32_t indexA, int32_t indexB);

	virtual alglm::vec3 supportA(const ConvexInfo &box, alglm::vec3 dir) override;
	virtual alglm::vec3 supportB(const ConvexInfo &cylinder, alglm::vec3 dir) override;
	virtual void findCollisionPoints(const ConvexInfo &box, const ConvexInfo &cylinder, CollisionInfo &collisionInfo,
									 EpaInfo &epaInfo, SimplexArray &simplexArray) override;
};
} // namespace ale
