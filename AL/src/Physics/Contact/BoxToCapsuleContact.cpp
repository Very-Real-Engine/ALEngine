#include "Physics/Contact/BoxToCapsuleContact.h"

namespace ale
{
BoxToCapsuleContact::BoxToCapsuleContact(Fixture *fixtureA, Fixture *fixtureB, int32_t indexA, int32_t indexB)
	: Contact(fixtureA, fixtureB, indexA, indexB) {};

Contact *BoxToCapsuleContact::create(Fixture *fixtureA, Fixture *fixtureB, int32_t indexA, int32_t indexB)
{
	void *memory = PhysicsAllocator::m_blockAllocator.allocateBlock(sizeof(BoxToCapsuleContact));
	return new (static_cast<BoxToCapsuleContact *>(memory)) BoxToCapsuleContact(fixtureA, fixtureB, indexA, indexB);
}

alglm::vec3 BoxToCapsuleContact::supportA(const ConvexInfo &box, alglm::vec3 dir)
{
	float dotAxes[3] = {alglm::dot(box.axes[0], dir) > 0 ? 1.0f : -1.0f,
						alglm::dot(box.axes[1], dir) > 0 ? 1.0f : -1.0f,
						alglm::dot(box.axes[2], dir) > 0 ? 1.0f : -1.0f};

	alglm::vec3 point = box.center;
	for (int32_t i = 0; i < 3; ++i)
	{
		point += box.axes[i] * (dotAxes[i] * box.halfSize[i]);
	}

	return point;
}

alglm::vec3 BoxToCapsuleContact::supportB(const ConvexInfo &capsule, alglm::vec3 dir)
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

void BoxToCapsuleContact::findCollisionPoints(const ConvexInfo &box, const ConvexInfo &capsule,
											  CollisionInfo &collisionInfo, EpaInfo &epaInfo,
											  SimplexArray &simplexArray)
{
	// std::cout << "box to capsule!!!!\n";
	if (isCollideToHemisphere(capsule, -epaInfo.normal))
	{
		collisionInfo.normal[0] = epaInfo.normal;
		collisionInfo.seperation[0] = epaInfo.distance;

		if (alglm::dot(capsule.axes[0], collisionInfo.normal[0]) < 0)
		{
			alglm::vec3 hemisphereCenter = capsule.center + capsule.axes[0] * 0.5f * capsule.height;
			collisionInfo.pointB[0] = hemisphereCenter - collisionInfo.normal[0] * capsule.radius;
			collisionInfo.pointA[0] = collisionInfo.pointB[0] + collisionInfo.normal[0] * collisionInfo.seperation[0];
		}
		else
		{
			alglm::vec3 hemisphereCenter = capsule.center - capsule.axes[0] * 0.5f * capsule.height;
			collisionInfo.pointB[0] = hemisphereCenter - collisionInfo.normal[0] * capsule.radius;
			collisionInfo.pointA[0] = collisionInfo.pointB[0] + collisionInfo.normal[0] * collisionInfo.seperation[0];
		}
		++collisionInfo.size;
	}
	else
	{
		// std::cout << "edge side!!!\n";
		Face refFace, incFace;

		setBoxFace(refFace, box, epaInfo.normal);
		setCapsuleFace(incFace, capsule, -epaInfo.normal);

		ContactPolygon contactPolygon;
		computeContactPolygon(contactPolygon, refFace, incFace);

		buildManifoldFromPolygon(collisionInfo, refFace, incFace, contactPolygon, epaInfo);
	}
}
} // namespace ale