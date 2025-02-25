#include "Physics/Contact/SphereToCapsuleContact.h"

namespace ale
{
SphereToCapsuleContact::SphereToCapsuleContact(Fixture *fixtureA, Fixture *fixtureB, int32_t indexA, int32_t indexB)
	: Contact(fixtureA, fixtureB, indexA, indexB) {};

Contact *SphereToCapsuleContact::create(Fixture *fixtureA, Fixture *fixtureB, int32_t indexA, int32_t indexB)
{
	void *memory = PhysicsAllocator::m_blockAllocator.allocateBlock(sizeof(SphereToCapsuleContact));
	return new (static_cast<SphereToCapsuleContact *>(memory))
		SphereToCapsuleContact(fixtureA, fixtureB, indexA, indexB);
}

alglm::vec3 SphereToCapsuleContact::supportA(const ConvexInfo &sphere, alglm::vec3 dir)
{
	return sphere.center + dir * sphere.radius;
}

alglm::vec3 SphereToCapsuleContact::supportB(const ConvexInfo &capsule, alglm::vec3 dir)
{
	float dotResult = alglm::dot(dir, capsule.axes[0]);

	alglm::vec3 move(0.0f);

	if (dotResult > 0)
	{
		move = capsule.axes[0] * capsule.height * 0.5f;
	}
	else if (dotResult < 0)
	{
		move = -capsule.axes[0] * capsule.height * 0.5f;
	}

	return capsule.center + move + dir * capsule.radius;
}

void SphereToCapsuleContact::findCollisionPoints(const ConvexInfo &sphere, const ConvexInfo &capsule,
												 CollisionInfo &collisionInfo, EpaInfo &epaInfo,
												 SimplexArray &simplexArray)
{
	collisionInfo.normal[0] = epaInfo.normal;
	collisionInfo.seperation[0] = epaInfo.distance;
	collisionInfo.pointA[0] = sphere.center + epaInfo.normal * sphere.radius;
	collisionInfo.pointB[0] = collisionInfo.pointA[0] - collisionInfo.normal[0] * collisionInfo.seperation[0];
	++collisionInfo.size;
}
} // namespace ale