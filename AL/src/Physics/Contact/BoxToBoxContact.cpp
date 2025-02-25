#include "Physics/Contact/BoxToBoxContact.h"

namespace ale
{
BoxToBoxContact::BoxToBoxContact(Fixture *fixtureA, Fixture *fixtureB, int32_t indexA, int32_t indexB)
	: Contact(fixtureA, fixtureB, indexA, indexB) {};

Contact *BoxToBoxContact::create(Fixture *fixtureA, Fixture *fixtureB, int32_t indexA, int32_t indexB)
{
	void *memory = PhysicsAllocator::m_blockAllocator.allocateBlock(sizeof(BoxToBoxContact));
	return new (static_cast<BoxToBoxContact *>(memory)) BoxToBoxContact(fixtureA, fixtureB, indexA, indexB);
}

alglm::vec3 BoxToBoxContact::supportA(const ConvexInfo &box, alglm::vec3 dir)
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

alglm::vec3 BoxToBoxContact::supportB(const ConvexInfo &box, alglm::vec3 dir)
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

void BoxToBoxContact::findCollisionPoints(const ConvexInfo &boxA, const ConvexInfo &boxB, CollisionInfo &collisionInfo,
										  EpaInfo &epaInfo, SimplexArray &simplexArray)
{
	// clipping
	Face refFace, incFace;
	setBoxFace(refFace, boxA, epaInfo.normal);
	setBoxFace(incFace, boxB, -epaInfo.normal);

	ContactPolygon contactPolygon;
	computeContactPolygon(contactPolygon, refFace, incFace);

	// 폴리곤의 각 꼭지점 -> 충돌점 여러 개
	buildManifoldFromPolygon(collisionInfo, refFace, incFace, contactPolygon, epaInfo);
}
} // namespace ale